#include <stdbool.h>
#include "lcd.h"
#include "system.h"
#include "radiation.h"
#include "rtc.h"
#include "adc.h"
#include "events.h"
#include "buzer.h"
#include "display.h"



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
	  //Initialize watchdog
	  iwdt_setup(IWDG_Prescaler_16,0xFFF);
}


/*----------------------------------------------------------*/

//Introduction and stabilization
static void introduction(void)
{
	//Write initial message
	lcd_printf("BoFF-R10");
	lcd_setpos(1,2);
	lcd_printf("ver0.11");
	//Enable buzzer
	buzer_alarm(true);
	//Wait a while
	timer_set(DISPLAY_TIMER,HZ);

	//Wait for timer
	while(timer_get(DISPLAY_TIMER))
	{
		iwdt_reset();
		wfi();
	}

	//Enable buzer alarm
	buzer_alarm(false);

}


/*----------------------------------------------------------*/

//Calculate global dose task
static void calculate_radiation_task(appState *app)
{

}

/*----------------------------------------------------------*/
//Check battery state
static void battery_check_task(appState *app)
{

}


/*----------------------------------------------------------*/
//Keyboyard task
static void keyb_task(appState *app)
{

}

/*----------------------------------------------------------*/
//Global application state
static appState app;

/*----------------------------------------------------------*/
void main(void) __attribute__ ((noreturn));

//Main core function
void main(void)
{
  //Initialize system devices
  perhiph_init();

  //Introduction menu
  introduction();

  lcd_clear();
  lcd_setpos(1,1);
  lcd_printf("44");
  lcd_setpos(1,2);
  lcd_printf("uR/h");

  while(1) iwdt_reset();

  while(1)
  {
	  //Calculate dose task
	  calculate_radiation_task(&app);

	  //Batery check task
	  battery_check_task(&app);

	  //Keyboyard task
	  keyb_task(&app);

	  //LCD display task
	  if(timer_get(DISPLAY_TIMER)==0)
	  {
		  timer_set(DISPLAY_TIMER,DISPLAY_REFRESH);
		  disp_funcs[app.menu](&app);
	  }

	  //Reset watchdog
	  iwdt_reset();

	  //Switch CPU to idle mode
	  wfi();
  }
}

