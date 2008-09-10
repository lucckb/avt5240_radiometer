#include "stm32f10x_lib.h"
#include "lcd.h"
#include "system.h"
#include "radiation.h"



/*----------------------------------------------------------*/
//Main core function
int main(void)
{

  //Initialize system perhiperals
  system_setup();
	
  //Enable PWM generation
  lcd_pwm_setup();
	  
   //Initialize LCD
  lcdInit();
  
  
  //1 bit for preemtion priority
  nvic_priority_group(NVIC_PriorityGroup_1);
  
  //Sys tick handler preemtion priority to lower
  nvic_system_priority(SystemHandler_SysTick,1,0);
  
  //Setup system timer to 0,01 s
  systick_setup(10000);
    
  lcdPutStr("LiniaC");
  lcdSetPos(0x40);
  lcdPutStr("Linia2");
      
  //Enable standard counting algoritm
  setup_radiation(radiationCountMEDIUM);
  
  int radiation;

  while(1)
  {
    lcdSetPos(0x40);
    lcdPutStr("        ");
    radiation = get_radiation(radiationCURRENT);
    lcdSetPos(0x40);
    if(radiation==0)
    {
    	lcdPutStr("-----");
    }
    else
    {
    	lcdPutInt(radiation);
    	lcdPutStr("uRh");
    }
    /*
    lcdSetPos(0);
    lcdPutStr("        ");
    lcdSetPos(0);
    lcdPutInt(get_radiation(radiationLAST));
    lcdPutStr("uRh");
    */
    systick_wait(HZ/5);
  } 
}

