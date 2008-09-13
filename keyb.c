/* Keyboyard driver for dozymeter    *
 * Copyright (c) Lucjan Bryndza 2008 *
 * All rights reserved               */

#include "stm32f10x_lib.h"
#include "keyb.h"

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
void on_keyb_timer_event(void)
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
