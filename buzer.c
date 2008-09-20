#include <stdbool.h>
#include <stdint.h>
#include "stm32f10x_lib.h"
#include "buzer.h"
#include "events.h"

/*----------------------------------------------------------*/
#define CCER_CC1E_Reset             ((u16)0x3332)
#define CCER_CC2E_Reset				(~(1<<4))             
#define CCER_CC1E_Set               ((u16)0x0001)
#define CCER_CC2E_Set               (1<<4)
#define CCER_CC1P_Reset             ((u16)0x3331)
#define CCER_CC2P_Reset             ~(1<<5)
#define TIM_OCPreload_Enable        ((u16)0x0008)
#define CR1_ARPE_Set                ((u16)0x0080)
#define CR1_CEN_Set                 ((u16)0x0001)

#define GPIO_BIT6_CRL_MASK 0xf0ffffff
#define GPIO_CNF_ALT_PUSHPULL 2
#define GPIO_MODE_10MHZ 1


//Enable 20kHZ signal voltage dubler for LCD power supply
void lcd_pwm_setup(void)
{
	//Enable CLK for TIM3
	RCC->APB1ENR |= RCC_APB1Periph_TIM3;
	//Timer base period
	TIM3->ARR = 199; 
	//Timer prescaler
	TIM3->PSC = 9;
	//Setup Control register
	TIM3->CR1 = 0;
	// Disable the Channel 1: Reset the CCE Bit 
	TIM3->CCER &= CCER_CC1E_Reset;
	//Setup PWM1 mode for channel 1
	TIM3->CCMR1 &= 0xff00;
	TIM3->CCMR1 |= TIM_OCMode_PWM1;
	// Set the Capture Compare Register value (Duty cycle 50%)
	TIM3->CCR1 = 100;
	//Compare channel 1 polarity high
	TIM3->CCER &= CCER_CC1P_Reset;
	//Enable compare chanel 1
	TIM3->CCER |= CCER_CC1E_Set;
	//Enable shadow preload register
	TIM3->CCMR1 |= TIM_OCPreload_Enable;
	//Auto reload register is buffered
	TIM3->CR1 |= CR1_ARPE_Set;
	//Enable gpio PWM line
	RCC->APB2ENR |= RCC_APB2Periph_GPIOA;
	//Max PA.6 signal
	GPIOA->CRL &= GPIO_BIT6_CRL_MASK;
	GPIOA->CRL |= (GPIO_MODE_10MHZ << 24) | (GPIO_CNF_ALT_PUSHPULL<<26);
	
	//Setup PWM for buzzer
	// Disable the Channel 2: Reset the CCE Bit
	TIM3->CCER &= CCER_CC2E_Reset;
	//Enable PWM1 mode
	TIM3->CCMR1 &= 0x00ff;
	TIM3->CCMR1 |= TIM_OCMode_PWM1<<8;
	//PWM duty cycle to 0%
	TIM3->CCR2 = 0;		//Disable signal PWM 0%
	//Reset polarity
	TIM3->CCER &= CCER_CC2P_Reset;
	TIM3->CCMR1 |= TIM_OCPreload_Enable<<8;
	//Enable timer
	TIM3->CR1 |= CR1_CEN_Set;
	
	//Buzer in GPIO mode control
	GPIOA->CRL &= 0x0fffffff;
	GPIOA->CRL |= (GPIO_MODE_10MHZ<<28);
}

/*----------------------------------------------------------*/
//Private variable buzzer state
static volatile bool buzerAlarm;

/*----------------------------------------------------------*/
//Private macros buzzer on of
#define BUZ_ON() TIM3->CCR2 = 100

#define BUZ_OFF() TIM3->CCR2 = 0


/*----------------------------------------------------------*/
//Setup buzzer to selected rate
void buzer_alarm(bool onOff)
{
	if(onOff)
	{
		TIM3->CCER |= CCER_CC2E_Set;
		GPIOA->CRL &= 0x0fffffff;
		GPIOA->CRL |= (GPIO_MODE_10MHZ<<28) |(GPIO_CNF_ALT_PUSHPULL<<30);
		//Enable timer
		TIM3->CR1 |= CR1_CEN_Set;
		buzerAlarm = 1;
	}
	else
	{
		GPIOA->CRL &= 0x0fffffff;
		GPIOA->CRL |= (GPIO_MODE_10MHZ<<28);
		// Disable the Channel 2: Reset the CCE Bit
		TIM3->CCER &= CCER_CC2E_Reset;
		buzerAlarm = 0;
	}
}

/*----------------------------------------------------------*/
//Internal buzzer timer event
void on_buzzer_timer_event(void)
{
	static uint16_t buzTim;
	if(buzerAlarm==0) return;
	if(buzTim--==0)
	{
		BUZ_OFF();
		buzTim = HZ/2;
	}
	else if(buzTim==HZ/4)
	{
		BUZ_ON();
	}
}
