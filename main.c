#include "stm32f10x_lib.h"
#include "lcd.h"


GPIO_InitTypeDef GPIO_InitStructure;
ErrorStatus HSEStartUpStatus;
TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
TIM_OCInitTypeDef  TIM_OCInitStructure;


void RCC_Configuration(void);
void NVIC_Configuration(void);
void Delay(vu32 nCount);


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


//Cortex stm32 clocks setup
static void system_setup(void)
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
    SCB->VTOR = 0;
    
}	


extern volatile int Tim;
extern volatile int Tim1;
//extern volatile int SysTick;

int main(void)
{

  /* Configure the system clocks */
  //RCC_Configuration();
  
  system_setup();
  
  /* NVIC Configuration */
 // NVIC_Configuration();

  /* Enable GPIOC clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    /* TIM3 clock enable */
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

  /* Configure PC.4 as Output push-pull */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

/* SysTick end of count event each 1ms with input clock equal to 9MHz (HCLK/8, default) */
  SysTick_SetReload(10000);

  /* Enable SysTick interrupt */
  SysTick_ITConfig(ENABLE);

  SysTick_CounterCmd(SysTick_Counter_Enable);

  lcdInit();
  lcdPutStr("LiniaZ");
  lcdSetPos(0x40);
  lcdPutStr("Linia2");
  
  //Enable PWM generation
  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = 199;
  TIM_TimeBaseStructure.TIM_Prescaler = 0;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

  /* PWM1 Mode configuration: Channel1 */
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
//  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = 100;
  TIM_OCInitStructure.TIM_Channel = TIM_Channel_1;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

  TIM_OCInit(TIM3, &TIM_OCInitStructure);

  TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);


  TIM_ARRPreloadConfig(TIM3, ENABLE);

  /*GPIOA Configuration: TIM3 channel 1 and 2 as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
    

  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* TIM3 enable counter */
  TIM_Cmd(TIM3, ENABLE);


  //For test counter only

  /* Time base configuration */
  /*
  TIM_TimeBaseStructure.TIM_Period = 65535;
  TIM_TimeBaseStructure.TIM_Prescaler = 0;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    */
      TIM2->SMCR = (1<<15) | (1<<14);
    TIM_Cmd(TIM2, ENABLE); 
    Tim1 = 4000; //40 second

    //Setup low power modes
    NVIC_SystemLPConfig(NVIC_LP_SLEEPONEXIT, DISABLE);
while(1)
{
    lcdSetPos(0x40);
    lcdPutStr("        ");
    lcdSetPos(0x40);
    lcdPutInt(TIM2->CNT);
    lcdPutStr("uRh");
    Tim=10;
    asm volatile("wfi");
    while(Tim);
    if(Tim1==0)
    {
        Tim1 = 4000;
        lcdSetPos(0);
        lcdPutStr("        ");
        lcdSetPos(0);
        lcdPutInt(TIM2->CNT);
        lcdPutStr("uRh");
        TIM2->CNT = 0;
    }
}
      while (1)
  {
    /* Turn on led connected to PC.4 pin */
    GPIO_SetBits(GPIOB, GPIO_Pin_12);
    /* Insert delay */
    //Delay(0xAFFFF);
    Tim = 100;
    while(Tim);
  
    //volatile float x;
    //x = x*12.3;
  
    /* Turn off led connected to PC.4 pin */
    GPIO_ResetBits(GPIOB, GPIO_Pin_12);
    /* Insert delay */
    //Delay(0xAFFFF);
    Tim = 100;
    while(Tim);
  }
}


void RCC_Configuration(void)
{
  /* RCC system reset(for debug purpose) */
  RCC_DeInit();

  /* Enable HSE */
  RCC_HSEConfig(RCC_HSE_ON);
  
  /* Wait till HSE is ready */
  HSEStartUpStatus = RCC_WaitForHSEStartUp();

  if(HSEStartUpStatus == SUCCESS)
	{
    /* Enable Prefetch Buffer */
    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

    /* Flash 2 wait state */
    FLASH_SetLatency(FLASH_Latency_0);
 	
    /* HCLK = SYSCLK */
    RCC_HCLKConfig(RCC_SYSCLK_Div1); 
  
    /* PCLK2 = HCLK */
    RCC_PCLK2Config(RCC_HCLK_Div1); 

    /* PCLK1 = HCLK/2 */
    RCC_PCLK1Config(RCC_HCLK_Div1);

    /* PLLCLK = 8MHz * 9 = 72 MHz */
    //RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);

    /* Enable PLL */ 
    //RCC_PLLCmd(ENABLE);

    /* Wait till PLL is ready */
    //while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
    //{
    //}

    /* Select PLL as system clock source */
    RCC_SYSCLKConfig(RCC_SYSCLKSource_HSE);

    /* Wait till PLL is used as system clock source */
    //while(RCC_GetSYSCLKSource() != 0x08)
    //{
    //}
  }
}


void NVIC_Configuration(void)
{
#ifdef  VECT_TAB_RAM  
  /* Set the Vector Table base location at 0x20000000 */ 
  NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0); 
#else  /* VECT_TAB_FLASH  */
  /* Set the Vector Table base location at 0x08000000 */ 
  NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);   
#endif
}


void Delay(vu32 nCount)
{
  for(; nCount != 0; nCount--);
}


