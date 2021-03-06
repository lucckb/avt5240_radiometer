/* *********************************************************** *
 * This is core module for radiation measure on SBM-20 GM tube *
 * This file is a part of Dozymeter. This file is owned by     *
 * Lucjan Bryndza. (c) 2008. All rights reserved               *
 * If you need licence contact to lucjan.bryndza@ep.com.pl     *
 * *********************************************************** */

#include "stm32f10x_lib.h"
#include "radiation.h"
#include "system.h"
#include "events.h"
#include "buzer.h"
#include <string.h>
/*----------------------------------------------------------*/

//Sample buf margin for computation
#define SAMPLEBUF_MARGIN 40

//Led timeout
#define LED_TIMEOUT 6

//Radiation samples count
#define LOW_SAMPLE_NUMB 40
#define MED_SAMPLE_NUMB 100
#define HI_SAMPLE_NUMB 400

//Samples buffer length
#define SAMPLEBUF_LENGTH (HI_SAMPLE_NUMB + SAMPLEBUF_MARGIN)

//Sample length for radiation
static volatile unsigned short samplesLength;

//Exposion dose last in standard alg
static volatile int radiationLast;

//High halfword of timer TIM2
static volatile unsigned short timerHi;


//Samples buffer for data
static volatile unsigned int samples[SAMPLEBUF_LENGTH+1];

//Current sample position counter
static volatile unsigned short samplesWrPos;


//Timer for standard algoritm
volatile short Tim40s = HZ*40;

/*----------------------------------------------------------*/
//Calculate current radiation
static int calcstd_radiation(void)
{
	uint32_t diff;
	uint64_t sum = 0;
	uint32_t n = 0;

	for(
		 uint32_t i=0,sWr=samplesWrPos,
		 i0=(sWr+samplesLength+SAMPLEBUF_MARGIN-1)%(samplesLength+SAMPLEBUF_MARGIN),
		 i1= (sWr+samplesLength+SAMPLEBUF_MARGIN-2)%(samplesLength+SAMPLEBUF_MARGIN) ;
		 i<samplesLength-1 ;
		 i++,
		 i0=(samplesLength+SAMPLEBUF_MARGIN-1+i0)%(samplesLength+SAMPLEBUF_MARGIN),
		 i1=(samplesLength+SAMPLEBUF_MARGIN-1+i1)%(samplesLength+SAMPLEBUF_MARGIN)
	    )
	{
		diff = samples[i0] - samples[i1];
		if(diff>10 && diff<8000000)
		{
			sum += diff;
			n++;
		}
	}
	//Too small samples count
    if(n<9) return 0;
    //Calculate dose
    return 8000000.0 / ( (float)sum / (float)n ) + 0.5 ;
}

/*----------------------------------------------------------*/
//Get radiation calculated in uR/h
int radiation_get(enum radiationMode mode)
{
	switch (mode)
	{
	//Get current expositive dose in standard algoritm
	case radiationCURRENT:
		if(samplesLength==0)
		{
			return radiationLast;
		}
		else
		{
			//Get uR/h in standard alghoritm
			return calcstd_radiation();
		}
	//Get last valid exp dose in standard algorithm
	case radiationCOUNTER:
		//Get uR/h in russian alghoritm
		return ((uint32_t)timerHi << 16) | TIM2->CNT;
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
#define CCER_CC1P (1<<1)				//Falling edge
#define CCMR1_CH1_MASK 0xff00			//Channel 1 mask
#define CCMR1_CH1_FILTER_F1_N4 2	    //Filter settings
#define CCMR1_CC1S_IC1_TI1 1			//Input mode
#define SR_CC1IF (1<<1)					//Comp/Cap CH1 pending bit
#define SR_UIF (1<<0)					//Update Tim pending bit
#define DIER_UIE (1<<0)					//Update interrupt register
#define CR1_URS (1<<2)					//Overflow update

#define GPIO_LED_Mask 0xfffff0ff		//GPIO Led mask
#define GPIO_MODE_10MHZ 1				//GpioMode 10M

/*----------------------------------------------------------*/
//Macro for LED control
#define LED_ON() GPIOA->BSRR = (1<<18)
#define LED_OFF() GPIOA->BSRR = (1<<2)

/*----------------------------------------------------------*/
//Initialize radiation at first time
void radiation_setup(void)
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

	//Gpio LED configuration
	GPIOA->CRL &= GPIO_LED_Mask;
	GPIOA->CRL |= GPIO_MODE_10MHZ<<8;
	LED_OFF();
}

/*----------------------------------------------------------*/
//Reconfigure radiation settings counter with standard russian counting alg.
void radiation_reconfigure(enum radiationCountMode mode)
{
	//Disable  nvic channel interrupt
	nvic_irq_enable(TIM2_IRQChannel,false);

	//Timer 2 reset signal
	RCC->APB1RSTR |= RCC_APB1Periph_TIM2;
	nop();
	RCC->APB1RSTR &= ~RCC_APB1Periph_TIM2;

	//Setup sample alghoritm
	switch(mode)
	{
		case radiationCountSTD:
			samplesLength = 0;
			break;
		case radiationCountSHORT:
			samplesLength = LOW_SAMPLE_NUMB;
			break;
		case radiationCountMEDIUM:
			samplesLength = MED_SAMPLE_NUMB;
			break;
		case radiationCountHIGH:
			samplesLength = HI_SAMPLE_NUMB;
			break;
	}


	//Reset current timer value
	timerHi = 0;
	TIM2->CNT = 0;

	//Standard counting based on russian devices
	if(mode==radiationCountSTD)
	{
		TIM2->PSC = 0;
		//Configure timer for counting external events
		TIM2->SMCR = SMCR_ETP | SMCR_ECE | (CCMR1_CH1_FILTER_F1_N4<<8);
		//Enable IRQ Overflow
		TIM2->DIER |= DIER_UIE;
		//Setup 40s counting timer
		Tim40s = HZ*40;
	}
	//Extended counting mode based on sample time
	else
	{
		//Erase memory with samples
		for(int i=0;i<SAMPLEBUF_LENGTH;i++) samples[i] = 0;
		//Reset sample WR position
		samplesWrPos = 0;
		//Prescaler for 5us pulse
		TIM2->PSC = 39;
		//Disable external counting mode
		TIM2->SMCR = 0;
		//Disable capture channel
		TIM2->CCER &= ~CCER_CC1E;
		//Setup CAP1 channel
		TIM2->CCMR1 &= CCMR1_CH1_MASK;
		//Filter length N=4 f=f
		TIM2->CCMR1 |= CCMR1_CH1_FILTER_F1_N4 << 4;
		//Select input from CH1
		TIM2->CCMR1 |= CCMR1_CC1S_IC1_TI1;
		//Enable capture channel falling edge
		TIM2->CCER |= CCER_CC1E | CCER_CC1P;

		//Enable IRQ capture channel 1
		TIM2->DIER |= DIER_CC1IE;
		//Enable IRQ Overflow
		TIM2->DIER |= DIER_UIE;
	}

	//Clear pending bit from IRQ
	nvic_irq_pend_clear(TIM2_IRQChannel);
	//Enable nvic channel
	nvic_irq_enable(TIM2_IRQChannel,true);
	//Enable timer2
	TIM2->CR1 |= CR1_CEN_Set;
}

/*----------------------------------------------------------*/

volatile short TimLed;

//Timer 2 exception handler
void timer2_handler(void) __attribute__((__interrupt__));
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
		//Blinking Led Algo
		LED_ON();
		TimLed = LED_TIMEOUT;
		//Click buzer
		buzer_click();
	}
}

/*----------------------------------------------------------*/


//This function should be called after 40s from interrupt handler
void on_radiation_timeout_event(void)
{
	//In standard alghoritm move measured radiation to dose
	if(--Tim40s==0)
	{
		//Update radiation last
		if(samplesLength==0)
		{
			radiationLast = ((uint32_t)timerHi << 16) | TIM2->CNT;
			TIM2->CNT = 0;
		}
		//Reload timer
		Tim40s = HZ * 40;
	}
	//Disable blink LED when event finished
	if(--TimLed==0) LED_OFF();
}
