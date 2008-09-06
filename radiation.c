/* *********************************************************** *
 * This is core module for radiation measure on SBM-20 GM tube *
 * This file is a part of Dozymeter. This file is owned by     *
 * Lucjan Bryndza. (c) 2008. All rights reserved               *
 * If you need licence contact to lucjan.bryndza@ep.com.pl     *
 * *********************************************************** */

#include "stm32f10x_lib.h"
#include "radiation.h"

/*----------------------------------------------------------*/
//Sample length for radiation 
static unsigned short samplesLength;

//Exposion dose last in standard alg
static volatile int expDoseLast; 

/*----------------------------------------------------------*/
//Get radiation calculated in uR/h
int get_radiation(enum radiationMode mode)
{
	switch (mode)
	{
	//Get current expositive dose in standard algoritm
	case radiationCURRENT:
		return TIM2->CNT;
	//Get last valid exp dose in standard algorithm
	case radiationLAST:
		return expDoseLast; 
	}
	return -1;
}

/*----------------------------------------------------------*/

#define GPIO_BIT0_CRL_MASK 0xfffffff0
#define GPIO_INPUT_FLOAT 1
#define CR1_CEN_Set                 ((u16)0x0001)

//Setup counter with standard russian counting alg.
void setup_radiation(enum radiationCountMode mode)
{
	
	//Enable APB perhiperal
	RCC->APB2ENR |= RCC_APB2Periph_GPIOA;
	RCC->APB1ENR |= RCC_APB1Periph_TIM2;
	//Setup GPIOA.0 as input
	GPIOA->CRL &= GPIO_BIT0_CRL_MASK;
	GPIOA->CRL |= GPIO_INPUT_FLOAT << 2;
	
	//Standard counting based on russian devices
	if(mode==radiationCountSTD)
	{
		//Configure timer for counting external events
		TIM2->SMCR = (1<<15) | (1<<14);
		//Enable timer
		TIM2->CR1 |= CR1_CEN_Set;
	}
	//Extended counting mode based on sample time
	else
	{
		//TODO: Fill this
	}
	samplesLength = mode;
	
}

/*----------------------------------------------------------*/
//This function should be called after 40s from interrupt handler
void radiation_on40s_timeout_event()
{
	if(samplesLength==0)
	{
		expDoseLast = TIM2->CNT;
		TIM2->CNT = 0;
	}
}
