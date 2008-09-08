/* *********************************************************** *
 * This is core module for radiation measure on SBM-20 GM tube *
 * This file is a part of Dozymeter. This file is owned by     *
 * Lucjan Bryndza. (c) 2008. All rights reserved               *
 * If you need licence contact to lucjan.bryndza@ep.com.pl     *
 * *********************************************************** */

#include "stm32f10x_lib.h"
#include "radiation.h"
#include "system.h"
/*----------------------------------------------------------*/

//Sample buf margin for computation
#define SAMPLEBUF_MARGIN 40

//Samples buffer length 
#define SAMPLEBUF_LENGTH (radiationCountHIGH + SAMPLEBUF_MARGIN)

//Sample length for radiation 
static unsigned short samplesLength;

//Exposion dose last in standard alg
static volatile int expDoseLast; 

//High halfword of timer TIM2
static volatile unsigned short timerHi;

//Samples buffer for data
static volatile unsigned int samples[SAMPLEBUF_LENGTH+1];

//Current sample position counter
static volatile unsigned short samplesWrPos;

/*----------------------------------------------------------*/
//Calculate current radiation
static int calcstd_radiation(void)
{
	uint32_t i0,i1,diff;
	short n = 0;
	uint64_t sum = 0;
	//Save sample in variable
	i1 = samplesWrPos;
	i0 = (i1 - 1)%(samplesLength+SAMPLEBUF_MARGIN);
	i1 = (i1 - samplesLength)%(samplesLength+SAMPLEBUF_MARGIN);
	for(int i=i0;i!=i1;i=(i-1)%(samplesLength+SAMPLEBUF_MARGIN))
	{
		lcdSetPos(0x40);
		lcdPutStr("     ");
		lcdSetPos(0x40);
		lcdPutInt(i);
		for(volatile int z=0;z<1000000;z++) asm volatile("nop");
		diff = samples[i] - samples[(i-1)%(samplesLength+SAMPLEBUF_MARGIN)];
		if(diff>5 && diff<2000000)
		{
			sum += diff;
			n++;
		}	
	}
	//Too small samples count
	if(n<10) return -1;
	float res = 40.0 / (((float)diff/(float)n)*5E-6);
	return res + 0.5;
}

/*----------------------------------------------------------*/
//Get radiation calculated in uR/h
int get_radiation(enum radiationMode mode)
{
	switch (mode)
	{
	//Get current expositive dose in standard algoritm
	case radiationCURRENT:
		if(samplesLength==0)
		{
			//Get uR/h in russian alghoritm
			return ((uint32_t)timerHi << 16) | TIM2->CNT;
		}
		else
		{
			//Get uR/h in standard alghoritm
			return calcstd_radiation();
		}
	//Get last valid exp dose in standard algorithm
	case radiationLAST:
		return expDoseLast; 
	}
	return -1;
}

/*----------------------------------------------------------*/

#define GPIO_BIT0_CRL_MASK 0xfffffff0   //GPio configuration mask
#define GPIO_INPUT_FLOAT 1
#define CR1_CEN_Set ((u16)0x0001)		//Counter enable
#define SMCR_ETP (1<<15)				//ETP bit
#define SMCR_ECE (1<<14)				//ECE Bit
#define DIER_CC1IE (1<<1)				//Enable interrupt compare capt
#define CCER_CC1E (1<<0)				//Enable compare capture CH1
#define CCMR1_CH1_MASK 0xff00			//Channel 1 mask
#define CCMR1_CH1_FILTER_F1_N2 (1<<4)	//Filter settings
#define CCMR1_CC1S_IC1_TI1 1			//Input mode
#define SR_CC1IF (1<<1)					//Comp/Cap CH1 pending bit
#define SR_UIF (1<<0)					//Update Tim pending bit
#define DIER_UIE (1<<0)					//Update interrupt register
#define CR1_URS (1<<2)					//Overflow update 

/*----------------------------------------------------------*/
//Setup counter with standard russian counting alg.
void setup_radiation(enum radiationCountMode mode)
{
	
	//Disable  nvic channel interrupt
	nvic_irq_enable(TIM2_IRQChannel,false);
	//Setup higest priority for this int
	nvic_irq_priority(TIM2_IRQChannel,0,0);
	
	//Enable APB perhiperal
	RCC->APB2ENR |= RCC_APB2Periph_GPIOA;
	RCC->APB1ENR |= RCC_APB1Periph_TIM2;
	//Setup GPIOA.0 as input
	GPIOA->CRL &= GPIO_BIT0_CRL_MASK;
	GPIOA->CRL |= GPIO_INPUT_FLOAT << 2;
	
	//Disable and reset timer
	TIM2->CR1 = CR1_URS;
	//Disable interrupt
	TIM2->DIER = 0;
	
	//Reset current timer value
	timerHi = 0;
	
	//Standard counting based on russian devices
	if(mode==radiationCountSTD)
	{
		//Disable capture channel
		TIM2->CCER &= ~CCER_CC1E;
		TIM2->PSC = 0;
		//Configure timer for counting external events
		TIM2->SMCR = SMCR_ETP | SMCR_ECE;
		//Enable IRQ Overflow
		TIM2->DIER |= DIER_UIE;
	}
	//Extended counting mode based on sample time
	else
	{
		//Prescaler for 5us pulse
		TIM2->PSC = 39;
		//Disable external counting mode
		TIM2->SMCR = 0;
		//Disable capture channel
		TIM2->CCER &= ~CCER_CC1E;
		//Setup CAP1 channel
		TIM2->CCMR1 &= CCMR1_CH1_MASK;
		//Filter length N=2
		TIM2->CCMR1 |= CCMR1_CH1_FILTER_F1_N2;
		//Select input from CH1
		TIM2->CCMR1 |= CCMR1_CC1S_IC1_TI1;
		//Enable capture channel
		TIM2->CCER |= CCER_CC1E;
		
		//Enable IRQ capture channel 1
		TIM2->DIER |= DIER_CC1IE;
		//Enable IRQ Overflow
		TIM2->DIER |= DIER_UIE;
	}
	//Setup sample alghoritm
	samplesLength = mode;
	//Enable nvic channel
	nvic_irq_enable(TIM2_IRQChannel,true);
	//Enable timer2
	TIM2->CR1 |= CR1_CEN_Set;
}

/*----------------------------------------------------------*/
//Timer 2 exception handler 
void timer2_handler(void)
{
	if(TIM2->SR & SR_UIF)
	{
		//Event overflow clear flag
		TIM2->SR &= ~SR_UIF;
		timerHi++;
	}
	if(TIM2->SR & SR_CC1IF)
	{
		//Capture event flag clear
		TIM2->SR &= ~SR_CC1IF;
		//Counting alghoritm
		samples[samplesWrPos++] = ((uint32_t)timerHi << 16) | TIM2->CCR1;
		if(samplesWrPos >= (samplesLength+SAMPLEBUF_MARGIN))
		{
			samplesWrPos = 0;
		}
		
	}
}

/*----------------------------------------------------------*/
//This function should be called after 40s from interrupt handler
void radiation_on40s_timeout_event(void)
{
	if(samplesLength==0)
	{
		expDoseLast = TIM2->CNT;
		TIM2->CNT = 0;
	}
}
