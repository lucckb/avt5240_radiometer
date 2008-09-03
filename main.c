#include "stm32f10x_lib.h"
#include "lcd.h"
#include "system.h"


GPIO_InitTypeDef GPIO_InitStructure;

TIM_OCInitTypeDef  TIM_OCInitStructure;



extern volatile int Tim;
extern volatile int Tim1;
//extern volatile int SysTick;

int main(void)
{
  
  
	//Initialize system perhiperals
	system_setup();
	
	
	//Initialize LCD
	lcdInit();
	

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);


  
  //Setup system timer to 0,01 s
  systick_setup(10000);
    
  lcdPutStr("LiniaO");
  lcdSetPos(0x40);
  lcdPutStr("Linia2");
  
  //Enable PWM generation
  /* Time base configuration */
  lcd_pwm_setup();
  


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

while(1)
{
    lcdSetPos(0x40);
    lcdPutStr("        ");
    lcdSetPos(0x40);
    lcdPutInt(TIM2->CNT);
    lcdPutStr("uRh");
    Tim=10;
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
      
}



