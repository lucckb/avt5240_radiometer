#ifndef SYSTEM_H_
#define SYSTEM_H_

/*----------------------------------------------------------*/

#include <stdint.h>
#include <stdbool.h>
#include "stm32f10x_lib.h"

/*----------------------------------------------------------*/
//System Hz 
#define HZ 100

//Stadard status
#define EXIT_SUCCESS 0
#define EXIT_FAILURE -1


/*----------------------------------------------------------*/
//Cortex stm32 System setup
void system_setup(void);

/*----------------------------------------------------------*/

//Configure system timer to requied interval
void systick_setup(int reload);

/*----------------------------------------------------------*/
//Setup buzzer to selected rate
void buzer_alarm(bool onOff);

/*----------------------------------------------------------*/
//Generate buzer click
static inline void buzer_click(void)
{
	//buzer_control(buzerPulse);
	GPIOA->ODR ^= (1<<7);
}

/*----------------------------------------------------------*/

//Enable 20kHZ signal voltage dubler for LCD power supply
void lcd_pwm_setup(void);

/*----------------------------------------------------------*/
//Internal use only definition
extern volatile int sysTimer;

/*----------------------------------------------------------*/
//System timer set value 
static inline void systick_set(int tick) { sysTimer = tick; }

/*----------------------------------------------------------*/
//System timer get value
static inline int systick_get(void) { return sysTimer; } 

/*----------------------------------------------------------*/
//Wait for selected time
static inline void systick_wait(int tick) { sysTimer = tick; while(sysTimer); }

/*----------------------------------------------------------*/
//Setup NVIC priority group 
void nvic_priority_group(uint32_t group);

/*----------------------------------------------------------*/
/* Setup NVIC priority
 * channel - setup selected channel
 * priority - assign IRQ preemtion priority
 * subpriority - assign supbriority */
void nvic_irq_priority(uint8_t channel,uint8_t priority,uint8_t subpriority);

/*----------------------------------------------------------*/
/* Enable or disable selected channel in NVIC
 * channel - channel number
 * enable - enable or disable */
void nvic_irq_enable(uint8_t channel, bool enable);

/*----------------------------------------------------------*/
/* Setup NVIC system handler priority
 * handler - setup selected channel
 * priority - assign IRQ preemtion priority
 * subpriority - assign supbriority */
void nvic_system_priority(uint32_t handler,uint8_t priority,uint8_t subpriority);

/*----------------------------------------------------------*/

#endif /*SYSTEM_H_*/
