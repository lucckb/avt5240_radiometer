#include "stm32f10x_lib.h"
#include "lcd.h"
#include "system.h"
#include "radiation.h"
#include "rtc.h"


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
  
  //Enable rtc
  rtc_setup();
  
  //Rtc test
  struct tm tm;
  memset(&tm,0,sizeof(tm));
  tm.tm_hour = 12;
  tm.tm_min = 12;
  tm.tm_sec = 12;
  tm.tm_mday = 10;
  tm.tm_mon = 9;
  tm.tm_year = 2008;
  
  
  //rtc_set(mktime(&tm));
  rtc_set(5555);
  
  lcdPutStr("BoFF");
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
    {
    	time_t t = rtc_get();
    	/*
    	gmtime_r(&t,&tm);
    	lcdSetPos(0);
    	lcdPutInt(tm.tm_hour);
    	lcdPutChar(':');
    	lcdPutInt(tm.tm_min);
    	lcdPutChar(':');
    	lcdPutInt(tm.tm_sec); */
    	lcdSetPos(0);
    	lcdPutInt(t);
    }
    systick_wait(HZ/5);
  } 
}

