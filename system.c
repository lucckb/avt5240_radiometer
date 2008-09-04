/* Microcontroller porting layer 
 * Lucjan Bryndza (c) 2008
 */

#include "stm32f10x_lib.h"
#include "system.h"


//External crystal on
#define RCC_CR_HSEON (1<<16)
//External bypass
#define RCC_CR_HSEBYP (1<<18)
//Crystal oscilator ready
#define RCC_CR_HSERDY (1<<17)
//Enable prefetch in flash CR
#define FLASH_ACR_PRFTBE 0x10
//Set flash latency to 0
#define FLASH_ACR_LATENCY_0 0
//High speed external as system clock
#define RCC_CFGR_SW_HSE 0x01
//MCO as system clock
#define RCC_CFGR_MCO_SYSCLK (4<<24)

//HSE oscilator control
#define RCC_CR_HSI_ON (1<<0)

/*----------------------------------------------------------*/
//Cortex stm32 System setup
void system_setup(void)
{
    //Configure CLK clock
    RCC->CR &= ~RCC_CR_HSEON;
    //Disable Bypass
    RCC->CR &= ~RCC_CR_HSEBYP;
    //Enable high speed oscilator
    RCC->CR  |= RCC_CR_HSEON;
    //Wait for setup HSE
    for(int i=0;i<8192;i++)
    {
        if(RCC->CR & RCC_CR_HSERDY) break;
    }
    //Configure flash: Prefetch enable and 0 wait state
    FLASH->ACR = FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY_0; 
    //Configure system clocks ALL clocks freq 8MHz
    RCC->CFGR = RCC_CFGR_SW_HSE | RCC_CFGR_MCO_SYSCLK;
    // At end disable HSI oscilator for power reduction
    RCC->CR &= ~RCC_CR_HSI_ON;
    //Setup NVIC vector at begin of flash
    SCB->VTOR = NVIC_VectTab_FLASH;
    
}	

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
#define CCER_CC1E_Reset             ((u16)0x3332)
#define TIM_OCMode_PWM1             ((u16)0x0060)
#define CCER_CC1E_Set               ((u16)0x0001)
#define CCER_CC1P_Reset             ((u16)0x3331)
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
	TIM3->PSC = 0;
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
	//Enable timer
	TIM3->CR1 |= CR1_CEN_Set;
}

/*----------------------------------------------------------*/
#define GPIO_BIT0_CRL_MASK 0xfffffff0
#define GPIO_INPUT_FLOAT 1

//Setup counter with standard russian counting alg.
void count_std_setup(void)
{
	//Enable APB perhiperal
	RCC->APB2ENR |= RCC_APB2Periph_GPIOA;
	RCC->APB1ENR |= RCC_APB1Periph_TIM2;
	//Setup GPIOA.0 as input
	GPIOA->CRL &= GPIO_BIT0_CRL_MASK;
	GPIOA->CRL |= GPIO_INPUT_FLOAT << 2;
	//Configure timer for counting external events
	TIM2->SMCR = (1<<15) | (1<<14);
	//Enable timer
	TIM2->CR1 |= CR1_CEN_Set;
}


