#include "rtc.h"
#include "stm32f10x_lib.h"
#include <stdint.h>
#include "system.h"

/*----------------------------------------------------------*/

//PWR control register
#define PWR_CR (*(volatile uint32_t*)0x40007000) 
//Enable access to backup domain and RTC
#define PWR_CR_DBP (1<<8)
//Enable LSE oscilator
#define BDCR_LSEON 1
//LSE ready flag
#define BDCR_LSERDY (1<<1)
//LSE clock source mask
#define BDCR_RTC_SRC_MASK (~((1<<8)|1<<9))
//Enable RTC clock bit
#define BDCR_RTC_EN (1<<15)
//Configuration Flag Enable Mask
#define CRL_CNF_Set      ((u16)0x0010)       
//Configuration Flag Disable Mask
#define CRL_CNF_Reset    ((u16)0xFFEF)       

/*----------------------------------------------------------*/
//Wait for sync
static void rtc_wait_for_sync(void)
{
	 // Clear RSF flag 
	  RTC->CRL &= ~RTC_FLAG_RSF;
	 // Loop until RSF flag is set 
	  while ((RTC->CRL & RTC_FLAG_RSF) == 0);
}

/*----------------------------------------------------------*/
//Wait for last write
static void rtc_wait_for_last_write(void)
{
	while( !(RTC->CRL & RTC_FLAG_RTOFF) );
}

/*----------------------------------------------------------*/
//Setup RTC clock
int rtc_setup(void)
{
	//Enable clock for power and backup domains
	RCC->APB1ENR |= RCC_APB1Periph_PWR | RCC_APB1Periph_BKP;
	//Enable acces to RTC and backup regs
	PWR_CR |= PWR_CR_DBP;
	//Disable LSE oscilator
	RCC->BDCR &= ~BDCR_LSEON;
	asm volatile("nop");
	//Enable LSE oscilator
	RCC->BDCR |= BDCR_LSEON;
	//Wait for lserdy flag
	int timeout=50000;
	while( !(RCC->BDCR & BDCR_LSERDY) )
		if(--timeout==0) return EXIT_FAILURE;
	//Enable LSE as RTC clock source
	RCC->BDCR &= BDCR_RTC_SRC_MASK;
	RCC->BDCR |= RCC_RTCCLKSource_LSE;
	//Enable RTC clock
	RCC->BDCR |= BDCR_RTC_EN;
	
	//Wait for synchronization
	rtc_wait_for_sync();
	//Wait for write
	rtc_wait_for_last_write();
	
	/* Enable configuration */
	 RTC->CRL |= CRL_CNF_Set;
	//RTC set prescaler 
	RTC->PRLH = 0;
	RTC->PRLL = 32768;
	/* Disable configuration */
    RTC->CRL &= CRL_CNF_Reset;
    
    //Wait for write
    rtc_wait_for_last_write();
    
    
	return EXIT_SUCCESS;
}

/*----------------------------------------------------------*/
//Get rtc clock
time_t rtc_get(void)
{
	return RTC->CNTL | ((uint32_t)RTC->CNTH)<<16;
}

/*----------------------------------------------------------*/

//Set rtc clock
void rtc_set(time_t time)
{
	rtc_wait_for_last_write();
	/* Enable configuration */
	RTC->CRL |= CRL_CNF_Set;
	//Set counter
	RTC->CNTH = time >> 16;
	RTC->CNTL = time;
	/* Disable configuration */
	RTC->CRL &= CRL_CNF_Reset;
}

/*----------------------------------------------------------*/
