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
//Calculate dose refresh
#define DOSE_REFRESH 60*HZ

//Radiation refresh
#define MAXRADIATION_REFRESH 5*HZ


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
//Max value when 3,3V reached
#define ADC_VMAX 24650
//Max ADC value
#define ADC_MAXVAL 4095


//Check battery state
static void battery_check_task(appState *app)
{
	static uint8_t cnt = 0;
	static uint32_t sum = 0;
    //Start conv and calculate conv 10 times

	//Total 10 measure
	if(cnt < 20)
	{
		if( (cnt & 1) == 0 )
		{
			//On odd bits start conv
			adc_startconv();
			timer_set(BATTERY_TIMER,2);
			cnt++;
		}
		else
		{
			//Od even get adc value
			if(timer_get(BATTERY_TIMER)==0)
			{
				sum += adc_getval();
				cnt++;
			}
		}
	}
	else if(cnt==20)
	{
		//If 10 get adc val wait 30s
		app->Vpwr = (((sum/10)*ADC_VMAX)/ADC_MAXVAL)/10;
		timer_set(BATTERY_TIMER,HZ*30);
		cnt++;
		sum = 0;
	}
	else
	{
		//On timeout start alghoritm again
		if(timer_get(BATTERY_TIMER)==0) cnt = 0;
	}
}


/*----------------------------------------------------------*/
//Keyboyard task
static void calc_radiation_task(appState *app)
{
	 //Add dose evry 1min
	 if(timer_get(DOSE_TIMER)==0)
	 {
		 timer_set(DOSE_TIMER,DOSE_REFRESH);
		 app->dose += get_radiation(radiationCURRENT);
	 }
	 //Max radiation every 5 seconds
	 if(timer_get(MAXRAD_TIMER)==0)
	 {

		//Timer radiation refresh
		timer_set(MAXRAD_TIMER,MAXRADIATION_REFRESH);
		//Get radiation
		int rad = get_radiation(radiationCURRENT);
		//Check for max value
		if(rad>app->radiationMax)
		{
			//Get radiation and get rtc time
			app->radiationMax = rad;
			app->radiationMaxTime = rtc_get();
		}
	 }
}

/*----------------------------------------------------------*/

//Global application state
static appState app =
{
		Vpwr: -1,
};

/*----------------------------------------------------------*/
void main(void) __attribute__ ((noreturn));

//Main core function
void main(void)
{
  //Initialize system devices
  perhiph_init();

  //Introduction menu
  introduction();

  setup_radiation(radiationCountMEDIUM);
  app.radiationAlgo = radiationCountMEDIUM;
  //app.unit = unitSI;

  //Setup dose timer
  timer_set(DOSE_TIMER,DOSE_REFRESH);
  //Setup radiation timer
  timer_set(MAXRAD_TIMER,MAXRADIATION_REFRESH);

  while(1)
  {

	  //Calculate radiation task
	  calc_radiation_task(&app);

	  //Batery check task
	  battery_check_task(&app);

	  //Keyboyard task
	  keyb_task(&app);

	  //LCD display task
	  if(timer_get(DISPLAY_TIMER)==0)
	  {
		  //On max radiation slow refresh for scroling
		  timer_set(DISPLAY_TIMER,HZ/4);
		  disp_funcs[app.menu](&app);
	  }

	  //Reset watchdog
	  iwdt_reset();

	  //Switch CPU to idle mode
	  wfi();
  }
}
