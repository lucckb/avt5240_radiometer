#ifndef SYSTEM_H_
#define SYSTEM_H_

/*----------------------------------------------------------*/

#include <stdint.h>
#include <stdbool.h>
#include "stm32f10x_lib.h"


/*----------------------------------------------------------*/

//Stadard status
#define EXIT_SUCCESS 0
#define EXIT_FAILURE -1
#define INVALID_VALUE -1;

/*----------------------------------------------------------*/
//Cortex stm32 System setup
void system_setup(void);

/*----------------------------------------------------------*/

//Setup NVIC priority group
void nvic_priority_group(uint32_t group);

/*----------------------------------------------------------*/
/* Setup NVIC priority
 * channel - setup selected channel
 * priority - assign IRQ preemtion priority
 * subpriority - assign supbriority */
void nvic_irq_priority(uint8_t channel,uint8_t priority,uint8_t subpriority);

/*----------------------------------------------------------*/
/* Enable or disable selected channel in NVIC
 * channel - channel number
 * enable - enable or disable */
void nvic_irq_enable(uint8_t channel, bool enable);

/*----------------------------------------------------------*/
/* Setup NVIC system handler priority
 * handler - setup selected channel
 * priority - assign IRQ preemtion priority
 * subpriority - assign supbriority */
void nvic_system_priority(uint32_t handler,uint8_t priority,uint8_t subpriority);

/*----------------------------------------------------------*/
//Memory access in bit band region

//Get address value from pointer
#define CADDR(x) ((unsigned long)&(x))

//Access to memory pointed by addr
#define MADDR(x) *((volatile unsigned long*)(x))

//Calculate bit band addr
#define BBAND(a,b) (((a)&0xF0000000)+0x02000000+(((a)&0xFFFFF)<<5)+((b)<<2))

//Access to bit addresable memory region
#define BITBAND(addr,bit) MADDR(BBAND(CADDR(addr),bit))

/*----------------------------------------------------------*/
//Sleep mode wait for interrupt macro

#ifndef PDEBUG
#define wfi() asm volatile("wfi")
#else
#define wfi()
#endif

/*----------------------------------------------------------*/
//Nop
#define nop() asm volatile("nop")

/*----------------------------------------------------------*/
/* Configure watchdog with selected
 * @param prescaler - prescaler value
 * @param reload - reload value */
void iwdt_setup(uint8_t prescaler,uint16_t reload);

/*----------------------------------------------------------*/

/* KR register bit mask */
#define KR_KEY_Reload    ((u16)0xAAAA)

//Reset wdt
#define iwdt_reset() IWDG->KR = KR_KEY_Reload

/*----------------------------------------------------------*/
//Try write atomic if success return 0
static inline long atomic_try_writeb(volatile uint8_t *addr,uint8_t val)
{
	long lock;
	asm volatile
	(
		"ldrexb %0,[%1]\n"
		"strexb %0,%2,[%1]\n"
		: "=&r"(lock)
		: "r"(addr),"r"(val)
		: "cc"
	);
	return lock;
}

/*----------------------------------------------------------*/

//Atomic exchange byte
static inline uint8_t atomic_xchg_byte(volatile uint8_t *addr,uint8_t val)
{
	uint8_t ret;
	unsigned long tmp;
	asm volatile
	(
	"1:	ldrexb %0,[%2]\n"
	   "strexb %1,%3,[%2]\n"
	   "teq %1,#0\n"
	   "bne 1b\n"
		: "=&r"(ret),"=&r"(tmp)
		: "r"(addr),"r"(val)
		: "cc"
	);
	return ret;
}

/*----------------------------------------------------------*/

#endif /*SYSTEM_H_*/
