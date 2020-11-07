/*
 *  @file  : timer.h
 *	
 *  @brief	
 *
 *  @author: franc
 *  @date  : 05 ott 2020
 */

#ifndef INC_TIMER_H_
#define INC_TIMER_H_

extern void vTimer_setSysTick(uint32_t timeMs);

extern void vTimer_2_init_OC(void);
extern void vTimer_5_init_IC(void);

extern void vTIM2_command(FunctionalState to_set);
extern void vTIM5_command(FunctionalState to_set);

#endif /* INC_TIMER_H_ */
