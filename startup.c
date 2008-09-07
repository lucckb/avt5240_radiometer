
//Cortex startup file based on STM32 code

/*----------------------------------------------------------*/
extern unsigned long _etext;
extern unsigned long _sidata;		/* start address for the initialization values
                                   of the .data section. defined in linker script */
extern unsigned long _sdata;		/* start address for the .data section. defined
                                   in linker script */
extern unsigned long _edata;		/* end address for the .data section. defined in
                                   linker script */

extern unsigned long _sbss;			/* start address for the .bss section. defined
                                   in linker script */
extern unsigned long _ebss;			/* end address for the .bss section. defined in
                                   linker script */

extern void _estack;				/* init value for the stack pointer. defined in linker script */
/*----------------------------------------------------------*/

void reset_handler(void) __attribute__((__interrupt__));

extern int main(void);

/*----------------------------------------------------------*/

//Handlers declarations
void sys_tick_handler(void);
void timer2_handler(void);

/*----------------------------------------------------------*/
//Unused vector dummy function
static void unused_vector() {}

/*----------------------------------------------------------*/
//Interrupt vector table
__attribute__ ((section(".isr_vector")))
void (* const exceptions_vectors[])(void) =
{
  &_estack,            // The initial stack pointer
  reset_handler,             // The reset handler
  unused_vector, //NMIException
  unused_vector, //HardFaultException
  unused_vector, //MemManageException
  unused_vector, //BusFaultException
  unused_vector, //UsageFaultException
  0, 0, 0, 0,            /* Reserved */ 
  unused_vector, //SVCHandler
  unused_vector, //DebugMonitor
  0,                      /* Reserved */
  unused_vector, 	//PendSVC
  sys_tick_handler,//SysTickHandler,
  unused_vector,//,WWDG_IRQHandler,
  unused_vector,//PVD_IRQHandler,
  unused_vector,//TAMPER_IRQHandler,
  unused_vector,//RTC_IRQHandler,TIM2->CCR1
  unused_vector,//FLASH_IRQHandler,
  unused_vector,//RCC_IRQHandler,
  unused_vector,//EXTI0_IRQHandler,
  unused_vector,//EXTI1_IRQHandler,
  unused_vector,//EXTI2_IRQHandler,
  unused_vector,//EXTI3_IRQHandler,
  unused_vector,//EXTI4_IRQHandler,
  unused_vector,//DMAChannel1_IRQHandler,
  unused_vector,//DMAChannel2_IRQHandler,
  unused_vector,//DMAChannel3_IRQHandler,
  unused_vector,//DMAChannel4_IRQHandler,
  unused_vector,//DMAChannel5_IRQHandler,
  unused_vector,//DMAChannel6_IRQHandler,
  unused_vector,//DMAChannel7_IRQHandler,
  unused_vector,//ADC_IRQHandler,
  unused_vector,//USB_HP_CAN_TX_IRQHandler,
  unused_vector,//USB_LP_CAN_RX0_IRQHandler,
  unused_vector,//CAN_RX1_IRQHandler,
  unused_vector,//CAN_SCE_IRQHandler,
  unused_vector,//EXTI9_5_IRQHandler,
  unused_vector,//TIM1_BRK_IRQHandler,
  unused_vector,//TIM1_UP_IRQHandler,
  unused_vector,//TIM1_TRG_COM_IRQHandler,
  unused_vector,//TIM1_CC_IRQHandler,
  timer2_handler,//TIM2_IRQHandler,
  unused_vector,//TIM3_IRQHandler,
  unused_vector,//TIM4_IRQHandler,
  unused_vector,//I2C1_EV_IRQHandler,
  unused_vector,//I2C1_ER_IRQHandler,
  unused_vector,//I2C2_EV_IRQHandler,
  unused_vector,//I2C2_ER_IRQHandler,
  unused_vector,//SPI1_IRQHandler,
  unused_vector,//SPI2_IRQHandler,
  unused_vector,//USART1_IRQHandler,
  unused_vector,//USART2_IRQHandler,
  unused_vector,//unused_vector,//USART3_IRQHandler,
  unused_vector,//EXTI15_10_IRQHandler,
  unused_vector,//RTCAlarm_IRQHandler,
  unused_vector,//USBWakeUp_IRQHandler,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  (void*)0xF108F85F //this is a workaround for boot in RAM mode.
};

/*----------------------------------------------------------*/
//Default reset handler
void reset_handler(void)
{
    unsigned long *pulSrc, *pulDest;

    //
    // Copy the data segment initializers from flash to SRAM.
    //
    pulSrc = &_sidata;
    for(pulDest = &_sdata; pulDest < &_edata; )
    {
        *(pulDest++) = *(pulSrc++);
    }

    //
    // Zero fill the bss segment.
    //
    for(pulDest = &_sbss; pulDest < &_ebss; )
    {
        *(pulDest++) = 0;
    }

    //
    // Call the application's entry point.
    //
    main();
}




