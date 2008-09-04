#include "stm32f10x_lib.h"
#include "lcd.h"
#include "system.h"



extern volatile int Tim;
extern volatile int Tim1;


int main(void)
{
  //Initialize system perhiperals
  system_setup();
	
  //Enable PWM generation
  lcd_pwm_setup();
	  
   //Initialize LCD
  lcdInit();
  
  //Setup system timer to 0,01 s
  systick_setup(10000);
    
  lcdPutStr("LiniaC");
  lcdSetPos(0x40);
  lcdPutStr("Linia2");
  
  //Enable standard counting algoritm
  count_std_setup();   
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

