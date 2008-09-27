#include <stdbool.h>
#include "lcd.h"
#include "system.h"
#include "radiation.h"
#include "rtc.h"
#include "adc.h"
#include "events.h"
#include "buzer.h"
#include "display.h"
#include "config.h"

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
	  //Enable AC converter
	  adc_setup();
	  //Initialize kbd
	  keyb_setup();

	  //Check for valid configuration
	  if(isvalid_config()==false)
	  {
		 if(rtc_setup()<0)
		 {
			 lcd_setpos(1,1);
			 lcd_printf("LSE osc ");
			 lcd_setpos(1,2);
			 lcd_printf("FAILED  ");
			 while(1);
		 }
		 //Write default time to rtc
		 rtc_set(DEFAULT_TIME_T);
	  }
	  else
	  {
		  //Init backup domain only
		  bkp_init();
	  }

	  //Initialize watchdog
	  iwdt_setup(IWDG_Prescaler_16,0xFFF);

	  //Initialize TIM2 for radiation
	  radiation_setup();
}


/*----------------------------------------------------------*/

//Introduction and stabilization
static void introduction(void)
{
	//Write initial message
	lcd_printf("BoFF-R10");
	lcd_setpos(1,2);
	lcd_printf("ver0.95");
	//Enable buzzer
	buzer_alarm(true);
	//Wait 1s
	timer_wait(HZ);
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
    static timer_t adc_timer = 0;
    static timer_t bat_timer= 0;

	//Start conv and calculate conv 10 times
	//Total 10 measure
	if(cnt < 20)
	{
		if( (cnt & 1) == 0 )
		{
			//On odd bits start conv
			adc_startconv();
			timer_start(adc_timer);
			cnt++;
		}
		else
		{
			//Od even get adc value
			if(timer_elapsed(adc_timer,2))
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
		timer_start(bat_timer);
		cnt++;
		sum = 0;
	}
	else
	{
		//On timeout start alghoritm again
		if(timer_elapsed(bat_timer,30*HZ)) cnt = 0;
	}
}

/*----------------------------------------------------------*/
//Keyboyard task
static void calc_radiation_task(appState *app)
{
	 //Dose timer
	 static timer_t dose_timer;
     //Maxrad timer
	 static timer_t maxrad_timer;
	 //Alarm timer
	 static timer_t alarm_timer;
	 //Add dose evry 1min
	 if(timer_elapsed(dose_timer,DOSE_REFRESH))
	 {
		 timer_start(dose_timer);
		 app->dose += radiation_get(radiationCURRENT);
	 }
	 //Max radiation every 5 seconds
	 if(timer_elapsed(maxrad_timer,MAXRADIATION_REFRESH))
	 {
		timer_start(maxrad_timer);
		//Get radiation
		int rad = radiation_get(radiationCURRENT);
		//Check for max value
		if(rad>app->radiationMax)
		{
			//Get radiation and get rtc time
			app->radiationMax = rad;
			app->radiationMaxTime = rtc_get();
			maximum_write_config(app);
		}
		//Check for alarm
		if(app->alarmLevel)
		{
			switch(app->alarmStat)
			{
				//Alarm is on
				case alarmON:
					if(timer_elapsed(alarm_timer,HZ*30))
					{
						//Disable alarm
						buzer_alarm(false);
						//Change state to alarm finished
						app->alarmStat = alarmFINISHED;
					}
					break;
				//Alarm finished
				case alarmFINISHED:
					if(rad < app->alarmLevel-2)
					{
						app->alarmStat = alarmNONE;
					}
					break;
				//Alarm is disabled
				case alarmNONE:
					if(rad>app->alarmLevel)
					{
						//Change state
						app->alarmStat = alarmON;
						//Enable alarm
						buzer_alarm(true);
						//Start timer
						timer_start(alarm_timer);
					}
					break;
			}

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

  //Read configuration from memory
  read_config(&app);

  //Setup radiation alghoritm
  radiation_reconfigure(app.radiationAlgo);

  //Display refresh timer
  static timer_t display_timer;

  while(1)
  {

	  //Calculate radiation task
	  calc_radiation_task(&app);

	  //Batery check task
	  battery_check_task(&app);

	  //Keyboyard task
	  keyb_task(&app);

	  //LCD display task
	  if(timer_elapsed(display_timer,HZ/4))
	  {
		  //Screen refresh time
		  timer_start(display_timer);
		  disp_funcs[app.menu](&app);
	  }

	  //Reset watchdog
	  iwdt_reset();

	  //Switch CPU to idle mode
	  wfi();
  }
}
