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
/* KR register bit mask */
#define KR_KEY_Reload    ((u16)0xAAAA)
#define KR_KEY_Enable    ((u16)0xCCCC)


/* Configure watchdog with selected 
 * @param prescaler - prescaler value
 * @param reload - reload value */

void iwdt_setup(uint8_t prescaler,uint16_t reload)
{
	//Enable write access to wdt
	IWDG->KR = IWDG_WriteAccess_Enable;
	//Program prescaler 
	IWDG->PR = prescaler;
	//Set reload value
	IWDG->RLR = reload;
	//Reload register
	IWDG->KR = KR_KEY_Reload;
	//Watchdog enable
	IWDG->KR = KR_KEY_Enable;
}

