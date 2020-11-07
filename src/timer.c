/*
 *  @file  : timer.c
 *	
 *  @brief	
 *
 *  @author: franc
 *  @date  : 05 ott 2020
 */

/* includes*/
#include "stm32f4xx.h"
#include "timer.h"

/* defines */

#define LED_PIN			GPIO_Pin_5
#define LED_PORT_		GPIOA
#define LED_PORT_CLK	RCC_AHB1Periph_GPIOA
#define LED_PIN_SOURCE	GPIO_PinSource5

#define INPUT_PIN			GPIO_Pin_0
#define INPUT_PIN_SRC		GPIO_PinSource0
#define INPUT_PORT			GPIOA
#define INPUT_CLOCK			RCC_AHB1Periph_GPIOA

/* Private data */
static volatile uint32_t sys_tick_timer = 0;


/* Private function prototype */

/* Public functions*/
void vTimer_setSysTick(uint32_t timeMs)
{
	 RCC_ClocksTypeDef RCC_Clocks;
	 RCC_GetClocksFreq(&RCC_Clocks);
	 SysTick_Config(RCC_Clocks.HCLK_Frequency/timeMs); // hz/s
}

/**
 *
 */
void vTimer_2_init_OC(void)
{
	GPIO_InitTypeDef 			GPIO_InitStructure;
	TIM_OCInitTypeDef 			TIM_OCInitStruct;
	RCC_ClocksTypeDef 			RCC_ClocksStatus;
	TIM_TimeBaseInitTypeDef 	TIM_TimeBaseInitStruct;
	NVIC_InitTypeDef 			NVIC_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	GPIO_InitStructure.GPIO_Pin = LED_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Fast_Speed;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	//GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(LED_PORT_, &GPIO_InitStructure);

	GPIO_PinAFConfig(LED_PORT_, LED_PIN_SOURCE, GPIO_AF_TIM2);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	RCC_GetClocksFreq(&RCC_ClocksStatus);
	uint16_t prescaler = ((RCC_ClocksStatus.PCLK1_Frequency)) / 1000  - 1; //1 tick = 1ms

	// tick freq = timer freq / (prescaler + 1)
	// tick freq = 84Mhz/ 84Mhz / 1000

	TIM_TimeBaseStructInit(&TIM_TimeBaseInitStruct);

	TIM_TimeBaseInitStruct.TIM_Prescaler = prescaler;
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStruct.TIM_Period = 10000;						/* period => 1ms*10000 = 1s*/
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStruct);	/* TIM 2 */


	TIM_OCStructInit(&TIM_OCInitStruct);
	TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;	/* why pwm1 and not TIM_OCMode_Active */
	TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStruct.TIM_Pulse = (2000); 							/* duty time = 5000*1ms = 5s*/
	TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OC1Init(TIM2, &TIM_OCInitStruct);

	TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);	/* write CCMR2 */
	//TIM_ARRPreloadConfig(TIM2, DISABLE);				/* set ARPE Auto Pre Load */

	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);				/* triggers when it reload */
}

/**
 *
 */
void vTimer_5_init_IC(void)
{
	GPIO_InitTypeDef 			GPIO_InitStructure;
	TIM_ICInitTypeDef 			TIM_ICInitStruct;
	RCC_ClocksTypeDef 			RCC_ClocksStatus;
	TIM_TimeBaseInitTypeDef 	TIM_TimeBaseInitStruct;
	NVIC_InitTypeDef 			NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
	RCC_AHB1PeriphClockCmd(INPUT_CLOCK, ENABLE);

	/* ------------------- pin configuration ------------------- */
	GPIO_InitStructure.GPIO_Pin = INPUT_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Fast_Speed;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_PinAFConfig(INPUT_PORT, INPUT_PIN_SRC, GPIO_AF_TIM5);

	/* ------------------- tim base configuration ------------------- */
	RCC_GetClocksFreq(&RCC_ClocksStatus);
	uint16_t prescaler = ((RCC_ClocksStatus.PCLK1_Frequency*2)) / 1000000 - 1; //1 tick = 1us (1 tick = 0.165mm resolution)

	TIM_TimeBaseStructInit(&TIM_TimeBaseInitStruct);

	TIM_TimeBaseInitStruct.TIM_Prescaler = prescaler;
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStruct.TIM_Period = 0xFFFF;						/* period ? */
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInit(TIM5, &TIM_TimeBaseInitStruct);	/* TIM 5 */

	/* ------------------- tim IC configuration ------------------- */

	/* set TIM5 channel 1 for input compare on RISING edge */
	TIM_ICStructInit(&TIM_ICInitStruct);
	TIM_ICInitStruct.TIM_Channel = TIM_Channel_1;
	TIM_ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_Rising;
	TIM_ICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInitStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM_ICInitStruct.TIM_ICFilter = 0;
	TIM_ICInit(TIM5, &TIM_ICInitStruct);

	/* set TIM5 channel 2 for input compare on FALLING edge */
	TIM_ICStructInit(&TIM_ICInitStruct);
	TIM_ICInitStruct.TIM_Channel = TIM_Channel_2;
	TIM_ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_Falling;
	TIM_ICInitStruct.TIM_ICSelection = TIM_ICSelection_IndirectTI;
	TIM_ICInitStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM_ICInitStruct.TIM_ICFilter = 0;
	TIM_ICInit(TIM5, &TIM_ICInitStruct);

	//TIM_PWMIConfig(TIM5, &TIM_ICInitStruct);

	/* select the  */
	TIM_SelectInputTrigger(TIM5, TIM_TS_TI1FP1);		/* set the input */
	TIM_SelectSlaveMode(TIM5 , TIM_SlaveMode_Reset);
	TIM_SelectMasterSlaveMode(TIM5 , TIM_MasterSlaveMode_Enable);

	/* ------------------- NVIC configuration ------------------- */
	NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	TIM_ITConfig(TIM5, TIM_IT_CC1, ENABLE);
	TIM_ITConfig(TIM5, TIM_IT_CC2, ENABLE);
}

/**
 *
 * @param to_set
 */
void vTIM2_command(FunctionalState to_set)
{
	TIM_Cmd(TIM2, to_set);				/* set CEN - Counter enable bit */
}

/**
 *
 * @param to_set
 */
void vTIM5_command(FunctionalState to_set)
{
	TIM_Cmd(TIM5, to_set);				/* set CEN - Counter enable bit */
}

/* Private functions */

/* Interrupt handler */
/**
 *
 */
void TIM5_IRQHandler(void)
{

}

/**
 *
 */
void TIM2_IRQHandler(void)
{
	uint32_t time;
	(void)time;

	if (TIM_GetITStatus(TIM2, TIM_IT_Update))
	{
		sys_tick_timer = 0;
		TIM2->CR1 &= (uint16_t)~TIM_CR1_CEN;
		time = TIM2->CCR1;
#if 0
		if ((time + 1000) < TIM2->ARR)
		{
			TIM_SetCompare1(TIM2 , (time + 1000));
		}
		else
		{
			TIM_SetCompare1(TIM2, 1000);
		}
#endif
		TIM2->CR1 |= TIM_CR1_CEN;
		TIM_ClearITPendingBit(TIM2, TIM_IT_CC1);
	}
}

void SysTick_Handler(void)
{
	sys_tick_timer++;
}
