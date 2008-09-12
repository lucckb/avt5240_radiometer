/* Microcontroller porting layer 
 * Lucjan Bryndza (c) 2008
 */


#include "stm32f10x_lib.h"
#include "system.h"
#include "radiation.h"

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
//Configure ADC prescaler to 8
#define RCC_CFGR_ADCPRE_8 (3<<14)

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
    //Configure system clocks ALL clocks freq 8MHz ADC to 1MHz
    RCC->CFGR = RCC_CFGR_SW_HSE | RCC_CFGR_MCO_SYSCLK | RCC_CFGR_ADCPRE_8;
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
//System timer handler called with frequency 100Hz
volatile int sysTimer = 0;
volatile short Tim40s = HZ*40;

void sys_tick_handler(void) __attribute__((__interrupt__));
void sys_tick_handler(void)
{
    if(sysTimer) --sysTimer;
    if(--Tim40s==0)
    {
    	//Called after 40s timeout event
    	radiation_on40s_timeout_event();
    	Tim40s = HZ * 40;
    }
}

/*----------------------------------------------------------*/
//Setup priority group in NVIC controler
#define AIRCR_VECTKEY_MASK    ((u32)0x05FA0000)

//Setup NVIC priority group 
void nvic_priority_group(uint32_t group)
{
	/* Set the PRIGROUP[10:8] bits according to NVIC_PriorityGroup value */
	  SCB->AIRCR = AIRCR_VECTKEY_MASK | group;

}
/*----------------------------------------------------------*/
/* Setup NVIC priority
 * channel - setup selected channel
 * priority - assign IRQ preemtion priority
 * subpriority - assign supbriority */
void nvic_irq_priority(uint8_t channel,uint8_t priority,uint8_t subpriority)
{
	/* Compute the Corresponding IRQ Priority */    
	uint32_t tmppriority = (0x700 - (SCB->AIRCR & (u32)0x700))>> 0x08;
	uint32_t tmppre = (0x4 - tmppriority);
	uint32_t tmpsub = 0x0F;
	tmpsub = tmpsub >> tmppriority;
	
	tmppriority = (uint32_t)priority << tmppre;
	tmppriority |=  subpriority & tmpsub;

	tmppriority = tmppriority << 0x04;
	tmppriority = ((u32)tmppriority) << ((channel & (u8)0x03) * 0x08);
	    
	uint32_t tmpreg = NVIC->IPR[(channel >> 0x02)];
	uint32_t tmpmask = (u32)0xFF << ((channel & (u8)0x03) * 0x08);
	tmpreg &= ~tmpmask;
	tmppriority &= tmpmask;  
	tmpreg |= tmppriority;
	NVIC->IPR[(channel >> 0x02)] = tmpreg;
}
/*----------------------------------------------------------*/
/* Enable or disable selected channel in NVIC
 * channel - channel number
 * enable - enable or disable */
void nvic_irq_enable(uint8_t channel, bool enable)
{
	if(enable)
	{
		/* Enable the Selected IRQ Channels */
		NVIC->ISER[channel >> 0x05] = (u32)0x01 << (channel & (u8)0x1F);
	}
	else
	{
		/* Disable the Selected IRQ Channels */
		NVIC->ICER[channel >> 0x05] = (u32)0x01 << (channel & (u8)0x1F);
	}
}

/*----------------------------------------------------------*/
/* Setup NVIC system handler priority
 * handler - setup selected channel
 * priority - assign IRQ preemtion priority
 * subpriority - assign supbriority */
void nvic_system_priority(uint32_t handler,uint8_t priority,uint8_t subpriority)
{
	uint32_t tmppriority = (0x700 - (SCB->AIRCR & (u32)0x700))>> 0x08;
	uint32_t tmp1 = (0x4 - tmppriority);
	uint32_t tmp2 = 0xff;
	tmp2 = tmp2 >> tmppriority;

	tmppriority = (u32)priority << tmp1;
	tmppriority |=  subpriority & tmp2;

	tmppriority = tmppriority << 0x04;
	tmp1 = handler & (u32)0xC0;
	tmp1 = tmp1 >> 0x06; 
	tmp2 = (handler >> 0x08) & (u32)0x03;
	tmppriority = tmppriority << (tmp2 * 0x08);
	uint32_t handlermask = (u32)0xFF << (tmp2 * 0x08);
	  
	SCB->SHPR[tmp1] &= ~handlermask;
	SCB->SHPR[tmp1] |= tmppriority;
}
/*----------------------------------------------------------*/
