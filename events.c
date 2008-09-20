/* **************************************************** *  
 * This file contains events based on timer and         * 
 *   keyboyard support. This is a part of dozymetr      *
 *   it is a proprietary product. All rights reserved   *
 *   Copyright Lucjan Bryndza <lucjan.bryndza@ep.com.pl *
 ********************************************************/

#include "stm32f10x_lib.h"
#include "system.h"
#include "radiation.h"
#include "events.h"
#include "buzer.h"
/*----------------------------------------------------------*/

#define CTRL_TICKINT_Set      		   ((u32)0x00000002)
#define SysTick_Counter_Enable         ((u32)0x00000001)

//System timer interrupt in microsecond interval 
void systick_setup(int reload)
{
	//Sys Tick reload value
	SysTick->LOAD = reload;
	//Sys tick enable interrupt
	SysTick->CTRL |= CTRL_TICKINT_Set;
	//System tick counter enable
	SysTick->CTRL |= SysTick_Counter_Enable;
}

/*----------------------------------------------------------*/
//Global timers
static volatile int gTimers[GENERIC_TIMERS];

//Events flagss
volatile unsigned long gEvents;

/*----------------------------------------------------------*/
//System timer set value 
void timer_set(int id,int tick) 
{ 
	if(id>=GENERIC_TIMERS) return;
	gTimers[id] = tick;
}

/*----------------------------------------------------------*/
//System timer get value
int timer_get(int id) 
{ 
	if(id>=GENERIC_TIMERS) return -1;
	return gTimers[id]; 
} 



/*----------------------------------------------------------*/
//Timer usage event 
static void on_timer_support_event(void)
{
	for(int i=0;i<GENERIC_TIMERS;i++)
	{
		if(gTimers[i]) --gTimers[i];
	}
}

/*----------------------------------------------------------*/

#define GPIO_CRH_B7_B5_Mask 0x000fffff
#define GPIO_MODE_PULLUP_INPUT 2
#define KEYPORT GPIOB

/*----------------------------------------------------------*/

//Keyboyard setup
void keyb_setup(void)
{
	//Enable analog ADC 
	RCC->APB2ENR |= RCC_APB2Periph_GPIOB;
	//Configure PB5..PB7 as input with Pullup
	KEYPORT->CRL &= GPIO_CRH_B7_B5_Mask;
	KEYPORT->CRL |= (GPIO_MODE_PULLUP_INPUT<<30) | 
    			  (GPIO_MODE_PULLUP_INPUT<<26) |
    			  (GPIO_MODE_PULLUP_INPUT<<22);
    //Pullup resistor
	KEYPORT->ODR |= (1<<5)|(1<<6)|(1<<7);
	
	
}

/*----------------------------------------------------------*/
//Last key pressed variable
volatile uint8_t keyb_key;

//This field is called from interrupt content
static void on_keyb_timer_event(void)
{
	//Obsluga klawiatury
		static uint8_t LastKey=0;
		static uint8_t RepeatK=0;
		static uint8_t DelayK=0;
		uint8_t PortKey;
		
		if(++RepeatK == 10)
		{
			RepeatK = 0;
			PortKey = (~KEYPORT->IDR >> 5)&0x07;
			if(PortKey)
			{
				if(PortKey == LastKey)
				{
					if(!DelayK)
					{	
						keyb_key = PortKey | KEYB_RPT;
					}
					else
					{
						--DelayK;
					}
				}
				else
				{
					keyb_key = LastKey = PortKey;
					DelayK = 15;
				}
			}
			else
			{
				LastKey =0;
			}
		}
}

/*----------------------------------------------------------*/
//System timer handler called with frequency 100Hz

void sys_tick_handler(void) __attribute__((__interrupt__));
void sys_tick_handler(void)
{
    
    //Called after 40s timeout event
    on_radiation_timeout_event();
    	
    //Keyboyard support
    on_keyb_timer_event();
    
    //Buzer timer event handler
    on_buzzer_timer_event();
    
    //On timer support event
    on_timer_support_event();
}

/*----------------------------------------------------------*/
