#include "lcd.h"
#include "system.h"
#include "radiation.h"
#include "rtc.h"
#include "stm32f10x_lib.h"
#include "adc.h"

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
  
  //Enable AC converter
  adc_setup();
  
  
  //Rtc test
  static struct rtc_tm tmx;
  
  tmx.tm_hour = 22;
  tmx.tm_isdst = 0;
  tmx.tm_mday = 1;
  tmx.tm_min = 34;
  tmx.tm_mon = 1;
  tmx.tm_sec = 0;
  tmx.tm_wday = 0;
  tmx.tm_yday = 1;
  tmx.tm_year = 2008 - 1900;
  
  //rtc_set(rtc_mktime(&tmx));
  
  lcdPutStr("BoFF");
  lcdSetPos(0x40);
  lcdPutStr("Linia2");
      
  //Enable standard counting algoritm
  setup_radiation(radiationCountMEDIUM);
 
  //rtc_bkp_write(1,1979);
  rtc_bkp_write(10,7711);
  
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
    /*
    {
    	time_t t = rtc_get();
    	rtc_gmtime(&t,&tmx);
    	lcdSetPos(0);
    	lcdPutInt(tmx.tm_hour);
    	lcdPutChar(':');
    	lcdPutInt(tmx.tm_min);
    	lcdPutChar(':');
    	lcdPutInt(tmx.tm_sec); 
    }*/
    adc_startconv();
    systick_wait(HZ/5);
    lcdSetPos(0);
    lcdPutInt(adc_getval());
  } 
}

