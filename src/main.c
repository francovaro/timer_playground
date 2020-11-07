/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/


#include "stm32f4xx.h"
#include "timer.h"

int main(void)
{
	vTimer_setSysTick(10);
	vTimer_2_init_OC();
	vTimer_5_init_IC();

	vTIM2_command(ENABLE);

	while(1)
	{

	}
}
