/*
 * display.c - All menu display functions
 *
 *  Created on: 2008-09-20
 *      Author: lucck
 */

#include <stdint.h>
#include <stdbool.h>
#include "lcd.h"
#include "display.h"
#include "radiation.h"
#include "events.h"
#include "config.h"

/*----------------------------------------------------------*/
//Global time tm structure
static struct rtc_tm tm;

/*----------------------------------------------------------*/
//Cursor position in data set alghoritm

enum {TIMESET_HOUR,TIMESET_MIN,TIMESET_SEC,
	  TIMESET_DAY,TIMESET_MONTH,TIMESET_YEAR };

/*----------------------------------------------------------*/


//Display current radiation
static void display_radiation(appState *app)
{
	static bool dispBat = false;

	//Set cursor to home position
	lcd_command(LCD_CURSOR_HOME);

	//Get radiation
	int rad = radiation_get(radiationCURRENT);

	if(app->radiationAlgo!= radiationCountSTD)
	{
		//Alghoritm 1/n
		if(app->unit==unitCLASSIC)
		{
			lcd_setpos(1,1);
			if(rad) lcd_printf("%u",rad);
			else lcd_printf("----");
			lcd_putspace(7);
			lcd_setpos(1,2);
			lcd_printf("uR/h");
		}
		else
		{
			lcd_setpos(1,1);
			if(rad) lcd_printf("%u.%02u",rad/100,rad%100);
			else lcd_printf("----");
			lcd_putspace(4);
			lcd_setpos(1,2);
			lcd_printf("uS/h");
		}
		//Disp bat status indicator
		if(dispBat)
		{
			if(app->Vpwr>400) dispBat = false;
		}
		else
		{
			if(app->Vpwr<380 && app->Vpwr !=-1) dispBat = true;
		}
		//Display status
		if(dispBat) lcd_printf(" Bat");
		else if(rad<79) lcd_printf(" Low");
		else if (rad>=79 && rad<=129) lcd_printf(" Med");
		else if(rad>129 && rad<=249) lcd_printf(" Hi ");
		else lcd_printf(" VHi");
	}
	else
	{
		//Russian standard alghoritm
		lcd_setpos(1,1);

		if(app->unit==unitCLASSIC)
		{
			lcd_printf("%uuRh",rad);
			lcd_putspace(4);
		}
		else
		{
			lcd_printf("%u.%02uuSh",rad/100,rad%100);
			lcd_putspace(1);
		}

		lcd_setpos(1,2);
		lcd_printf("%duRh",radiation_get(radiationCOUNTER));
		lcd_putspace(4);
	}
}

/*----------------------------------------------------------*/
//Display current radiation
static void display_max_radiation(appState *app)
{
	if(app->scrollPos==0)
	{
		if(app->radiationMax>0)
		{
			//Alghoritm 1/n
			if(app->unit==unitCLASSIC)
			{
				lcd_setpos(1,1);
				lcd_printf("Max=%uuR/h",app->radiationMax);
				lcd_putspace(10);
			}
			else
			{
				lcd_setpos(1,1);
				lcd_printf("Max=%u.%02uuS/h",app->radiationMax/100,app->radiationMax%100);
				lcd_putspace(7);
			}
			//Get time from rtc clock
			rtc_time(app->radiationMaxTime,&tm);
			lcd_setpos(1,2);
			lcd_printf("%02u:%02u:%02u ",tm.tm_hour,tm.tm_min,tm.tm_sec);
			lcd_printf("%02u-%02u-%04u",tm.tm_mday,tm.tm_mon,tm.tm_year+1900);
		}
		else
		{
			//If no radiation put default chars
			lcd_setpos(1,1);
			lcd_printf("********");
			lcd_putspace(8);
			lcd_setpos(1,2);
			lcd_printf("**:**:** **-**-****");
		}
	}
	else if(app->scrollPos<29 && app->scrollPos>6  && (app->scrollPos&1))
	{
		lcd_command(LCD_SHIFT_RIGHT);
	}
	if(app->scrollPos==37)
	{
		lcd_command(LCD_CURSOR_HOME);
		app->scrollPos = 0;
	}
	else
	{
		app->scrollPos++;
	}
}

/*----------------------------------------------------------*/
//Display dose rate
static void display_dose(appState *app)
{
	//Set cursor to home position
	lcd_command(LCD_CURSOR_HOME);
	//goto line 1
	lcd_setpos(1,1);

	//Current dose in uR/h
	uint64_t dose = app->dose / 60;

	if(app->unit==unitCLASSIC)
	{
		uint32_t dose_c = dose/1000;
		uint32_t dose_r = dose%1000;
		lcd_printf("%u.%03u",dose_c,dose_r);
		lcd_putspace(3);
	}
	else
	{
		uint32_t dose_c = dose/100000;
		uint32_t dose_r = dose%100000;
		lcd_printf("%u.%05u ",dose_c,dose_r);
	}
	//goto line 2
	lcd_setpos(1,2);
	//Write unit
	if(app->unit==unitCLASSIC)
	{
		lcd_printf("mrem   D");
	}
	else
	{
		lcd_printf("mSv    D");
	}
}

/*----------------------------------------------------------*/
//Display date from rtc
static void display_date(appState *app)
{
	rtc_time(rtc_get(),&tm);
	lcd_setpos(1,1);
	lcd_printf("%02d:%02d:%02d ",tm.tm_hour,tm.tm_min,tm.tm_sec);
	lcd_setpos(1,2);
	lcd_printf("%02d-%02d-%02d",tm.tm_mday,tm.tm_mon,tm.tm_year+1900-2000);
}

/*----------------------------------------------------------*/
//Display configuration
static void display_config (appState *app)
{
	lcd_setpos(1,1);
	lcd_printf("Config? ");
	lcd_setpos(1,2);
	lcd_printf("[C] [OK]");
}

/*----------------------------------------------------------*/

//Display alarm set
static void display_alarmset(appState *app)
{
	lcd_setpos(1,1);
	lcd_printf("ALARM   ");
	lcd_setpos(1,2);
	if(app->alarmLevel)
		lcd_printf("%d uR/h  ",app->alarmLevel);
	else
		lcd_printf("Disabled");
}

/*----------------------------------------------------------*/

//Display unit settings
static void display_unitset(appState *app)
{
	lcd_setpos(1,1);
	lcd_printf("UNIT    ");
	lcd_setpos(1,2);
	if(app->unit==unitCLASSIC) lcd_printf("STD uR/h");
	else lcd_printf("SI uS/h ");
}

/*----------------------------------------------------------*/

//Display alghoritm set
static void display_algoset(appState *app)
{
	//Alghoritm table
	static const char * const algo_descr[] =
	{
		"None    ",
		"Short   ",
		"Medium  ",
		"Long    "
	};
	//Display current algo settings
	lcd_setpos(1,1);
	lcd_printf("INTEGRAT");
	lcd_setpos(1,2);
	lcd_printf("%s",algo_descr[app->radiationAlgo]);
}

/*----------------------------------------------------------*/

//Display alghoritm set
static void display_dateset(appState *app)
{
	static const uint8_t cursor_pos[][2] =
	{
			{1,1},{4,1},{7,1},
			{1,2},{4,2},{7,2}
	};
	lcd_setpos(1,1);
	lcd_printf("%02d:%02d:%02d ",tm.tm_hour,tm.tm_min,tm.tm_sec);
	lcd_setpos(1,2);
	lcd_printf("%02d-%02d-%02d",tm.tm_mday,tm.tm_mon,tm.tm_year+1900-2000);
	lcd_setpos(cursor_pos[app->cursorPos][0],cursor_pos[app->cursorPos][1]);
}

/*----------------------------------------------------------*/
//Display delete dose
static void display_deldose(appState *app)
{
	lcd_setpos(1,1);
	lcd_printf("DEL dose");
	lcd_setpos(1,2);
	lcd_printf("[C] [OK]");
}

/*----------------------------------------------------------*/
//Display delete max
static void display_delmaxrad(appState *app)
{
	//Set cursor to home position
	lcd_command(LCD_CURSOR_HOME);
	lcd_setpos(1,1);
	lcd_printf("DEL max ");
	lcd_setpos(1,2);
	lcd_printf("[C] [OK]");
}

/*----------------------------------------------------------*/
//Display function pointers
const appfn_ptr disp_funcs[] =
{
		display_radiation,		//Display current radiation
		display_max_radiation,	//Display max radiation
		display_dose,			//Display dose
		display_date,			//Display date
		display_config,			//Display configuration
		display_alarmset,		//Alarm settings
		display_unitset,		//Display unit settings
		display_algoset,		//Display algo settings
		display_date,			//Display date in setting
		display_dateset,		//Display dateset
		display_deldose,		//Display delete dose
		display_delmaxrad,		//Display delmax
};

/*----------------------------------------------------------*/

//Forward declarations
static void keyb_configmenu(appState *app);
static void keyb_mainmenu(appState *app);
static void keyb_datemenu(appState *app);
static void keyb_yesno(appState *app);
/*----------------------------------------------------------*/

//Configure and enter date
static void keyb_datemenu(appState *app)
{
	uint8_t key = keyb_get();
	if(key==KEY_ENTER)
	{
		app->cursorPos++;
		if(app->cursorPos>TIMESET_YEAR)
			app->cursorPos=TIMESET_HOUR;
	}
	else if(key&KEY_NEXT)
	{
		switch(app->cursorPos)
		{
			case TIMESET_HOUR:
				if(++tm.tm_hour>23) tm.tm_hour = 0;
				break;
			case TIMESET_MIN:
				if(++tm.tm_min>59) tm.tm_min = 0;
				break;
			case TIMESET_SEC:
				if(++tm.tm_sec>59) tm.tm_sec = 0;
				break;
			case TIMESET_DAY:
				if(++tm.tm_mday>31) tm.tm_mday = 1;
				break;
			case TIMESET_MONTH:
				if(++tm.tm_mon>12) tm.tm_mon = 1;
				break;
			case TIMESET_YEAR:
				if(++tm.tm_year>(2099-1900)) tm.tm_year = 2000-1900;
				break;
		}
	}
	//Try setup date
	else if(key==KEY_ESC)
	{
		//Simple check for month day
		if(tm.tm_mon==2)
		{
			if(tm.tm_year%4==0)
			{
				if(tm.tm_mday>29) tm.tm_mday=29;
			}
			else
			{
				if(tm.tm_mday>28) tm.tm_mday=28;
			}
		}
		else if((tm.tm_mon&1)==0)
		{
			if(tm.tm_mday>30) tm.tm_mday=30;
		}
		time_t t = rtc_mktime(&tm);
		//Set time
		rtc_set(t);
		//Disable cursor
		lcd_command(LCD_CURSOR_HIDE);
		//Goto setup menu and it kbd handler
		app->menu = mnuSET_DATE;
		keyb_task = keyb_configmenu;
	}
}


/*----------------------------------------------------------*/
//Yes no decision
static void keyb_yesno(appState *app)
{
	uint8_t key = keyb_get();
	if(key==KEY_ENTER)
	{
		//Jezeli kasujemy dawke
		if(app->menu==mnuDISP_DELDOSE)
		{
			//Skasuj dawke i wyswietl dawke
			app->dose = 0;
			app->menu = mnuDISP_DOSE;
		}
		else if(app->menu==mnuDISP_DELMAXRAD)
		{
			//Skasuj maximum i do menu glownego
			app->radiationMax = 0;
			app->menu = mnuMAX_RADIATION;
			app->radiationMaxTime = DEFAULT_TIME_T;
			app->scrollPos = 0;
			//Write maximum data value
			maximum_write_config(app);
		}
		keyb_task = keyb_mainmenu;
	}
	else if(key==KEY_ESC)
	{
		//Do menu dawki
		if(app->menu==mnuDISP_DELDOSE)
		{
			app->menu = mnuDISP_DOSE;
		}
		//Do menu radiacji
		else if(app->menu==mnuDISP_DELMAXRAD)
		{
			app->menu = mnuMAX_RADIATION;
			app->scrollPos = 0;
		}
		keyb_task = keyb_mainmenu;
	}
}

/*----------------------------------------------------------*/

//Configuration menu
static void keyb_configmenu(appState *app)
{
	uint8_t key = keyb_get();
	if(key==KEY_NEXT) app->menu++;
	//Prevent return to main screen
	if(app->menu>mnuSET_DATE) app->menu = mnuSET_ALARM;
	if(key==KEY_ESC)
	{
		app->menu = mnuRADIATION;
		keyb_task = keyb_mainmenu;
	}
	else if(key==KEY_ENTER)
	{
		//Alarm mode change set
		if(app->menu==mnuSET_ALARM)
		{
			app->alarmLevel+=20;
			if(app->alarmLevel>200) app->alarmLevel = 0;
		}
		//Change unit
		else if(app->menu==mnuSET_UNIT)
		{
			//Change unit set
			if(app->unit==unitCLASSIC) app->unit=unitSI;
			else app->unit=unitCLASSIC;
			//Save settings
			settings_write_config(app);
		}
		//Change algoritm
		else if(app->menu==mnuSET_ALGO)
		{
			app->radiationAlgo++;
			if(app->radiationAlgo>radiationCountHIGH)
				app->radiationAlgo = radiationCountSTD;
			//Change radiation type
			radiation_reconfigure(app->radiationAlgo);
			//Save radiation config
			settings_write_config(app);
		}
		//Goto enter date
		else if(app->menu==mnuSET_DATE)
		{
			//Show cursor switch to display date
			lcd_command(LCD_CURSOR_SHOW);
			//Switch to enter date menu
			app->menu = mnuENTER_DATE;
			//Switch to new keyboyard handler
			keyb_task = keyb_datemenu;
			//Get current time from rtc
			rtc_time(rtc_get(),&tm);
			//Set cursor pos to 0
			app->cursorPos = 0;
		}
	}
}

/*----------------------------------------------------------*/
//Keyboard task
static void keyb_mainmenu(appState *app)
{
	uint8_t key = keyb_get();
	//Main menu switch keys
	if(key==KEY_NEXT)
	{
		//Increment menu position
		app->menu++;
		//Zero scrol varible
		app->scrollPos = 0;
		//If max reached start again
		if(app->menu==mnuSET_ALARM) app->menu=mnuRADIATION;
	}
	else if(key==KEY_ENTER)
	{
		//Disp delDose
		if(app->menu==mnuDISP_DOSE)
		{
			app->menu=mnuDISP_DELDOSE;
			keyb_task = keyb_yesno;
		}
		//Disp delMax
		else if(app->menu==mnuMAX_RADIATION)
		{
			app->menu=mnuDISP_DELMAXRAD;
			keyb_task = keyb_yesno;
		}
		//Disp enter conf
		else if(app->menu==mnuDISP_CONF)
		{
			//Goto setup menu and it kbd handler
			app->menu = mnuSET_ALARM;
			keyb_task = keyb_configmenu;
		}
	}
	//Escape key return to radiation
	else if(key==KEY_ESC)
	{
		app->menu = mnuRADIATION;
	}
}

/*----------------------------------------------------------*/

//Keyboard support function
appfn_ptr keyb_task = keyb_mainmenu;


/*----------------------------------------------------------*/
