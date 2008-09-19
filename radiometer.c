#include <stdbool.h>
#include "lcd.h"
#include "system.h"
#include "radiation.h"
#include "rtc.h"
#include "adc.h"
#include "events.h"
#include "buzer.h"


/*----------------------------------------------------------*/
//Initialize system perhiperals
static void perhiph_init(void)
{
	 //Initialize system perhiperals
	  system_setup();		
	  //Enable PWM generation
	  lcd_pwm_setup();   
	  //Initialize LCD
	  lcd_init();
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
	  //Initialize kbd
	  keyb_setup();  
}


/*----------------------------------------------------------*/

//Introduction and stabilization
static void introduction(void)
{
	//Write initial message
	lcd_printf("BF-R10");
	lcd_setpos(1,2);
	lcd_printf("ver0.1");
	//Enable buzzer 
	buzer_alarm(true);
	//Wait a while
	timer_set(DISPLAY_TIMER,HZ);
	
	//Wait a while
	while(timer_get(DISPLAY_TIMER)) wfi();
	
	//Enable buzer alarm
	buzer_alarm(false);
	
}

/*----------------------------------------------------------*/
//Main core function
int main(void)
{
  
  //Initialize system Perhipherals
  perhiph_init();
  
  //Introduction menu
  introduction();
  
  //Enable standard counting algoritm
  setup_radiation(radiationCountMEDIUM);
 
  //rtc_bkp_write(1,1979);
  rtc_bkp_write(10,7711);
  
  int radiation;
  
  
  while(1)
  {
    lcd_printf("        ");
    radiation = get_radiation(radiationCURRENT);
    lcd_setpos(1,2);
    if(radiation==0)
    {
    	lcd_printf("-----");
    }
    else
    {
    	lcd_printf("%duRh",radiation);
    }
    timer_set(DISPLAY_TIMER,HZ/5);
    while(timer_get(DISPLAY_TIMER)) wfi();
  }
}

