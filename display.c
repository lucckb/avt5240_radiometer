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

/*----------------------------------------------------------*/
//Display current radiation
static void display_radiation(appState *app)
{
	static bool dispBat = false;

	//Set cursor to home position
	lcd_command(LCD_CURSOR_HOME);

	//Get radiation
	int rad = get_radiation(radiationCURRENT);

	if(app->radiationAlgo!= radiationCountSTD)
	{
		//Alghoritm 1/n
		if(app->unit==unitCLASSIC)
		{
			lcd_setpos(1,1);
			if(rad) lcd_printf("%u       ",rad);
			else lcd_printf("----    ");
			lcd_setpos(1,2);
			lcd_printf("uR/h");
		}
		else
		{
			lcd_setpos(1,1);
			if(rad) lcd_printf("%u.%02u    ",rad/100,rad%100);
			else lcd_printf("----    ");
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
			lcd_printf("%uuRh    ",rad);
		else
			lcd_printf("%u.%uuSh  ",rad);

		lcd_setpos(1,2);
		lcd_printf("%duRh    ",get_radiation(radiationLAST));
	}
}

/*----------------------------------------------------------*/
//Display current radiation
static void display_max_radiation(appState *app)
{
	if(app->scrollPos==0)
	{
		//Alghoritm 1/n
		if(app->unit==unitCLASSIC)
		{
			lcd_setpos(1,1);
			lcd_printf("Max=%uuR/h",app->radiationMax);
		}
		else
		{
			lcd_setpos(1,1);
			lcd_printf("Max=%u.%02uuS/h",app->radiationMax/100,app->radiationMax%100);
		}
		//Get time from rtc clock
		struct rtc_tm tm;
		rtc_localtime(app->radiationMaxTime,&tm);
		lcd_setpos(1,2);
		lcd_printf("%02d:%02d:%02d ",tm.tm_hour,tm.tm_min,tm.tm_sec);
		lcd_printf("%02d-%02d-%04d",tm.tm_mday,tm.tm_mon,tm.tm_year);
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
		lcd_printf("%u.%03u   ",dose_c,dose_r);
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
static void display_dummy(appState *app){}


/*----------------------------------------------------------*/
//Display date from rtc
static void display_date(appState *app)
{
	struct rtc_tm tm;
	rtc_localtime(rtc_get(),&tm);
	lcd_setpos(1,1);
	lcd_printf("%02d:%02d:%02d ",tm.tm_hour,tm.tm_min,tm.tm_sec);
	lcd_setpos(1,2);
	lcd_printf("%02d-%02d-%04d",tm.tm_mday,tm.tm_mon,tm.tm_year);
}

/*----------------------------------------------------------*/
//Display function pointers
const appfn_ptr disp_funcs[] =
{
		display_radiation,		//Display current radiation
		display_max_radiation,	//Display max radiation
		display_dose,			//Display dose
		display_date,
		display_dummy,
		display_dummy,
		display_dummy,
		display_dummy
};


/*----------------------------------------------------------*/

//Keyboyard task
static void keyb_mainmenu(appState *app)
{
	uint8_t key = keyb_get();
	if(key==KEY_NEXT)
	{
		//Increment menu position
		app->menu++;
		//Zero scrol varible
		app->scrollPos = 0;
	}
	if(app->menu==mnuSET_ALARM) app->menu=mnuRADIATION;
	if(key==(KEY_ENTER|KEYB_RPT))
	{
		if(app->menu==mnuDISP_DOSE) app->dose = 0;
		if(app->menu==mnuMAX_RADIATION) app->radiationMax = 0;
	}
}

/*----------------------------------------------------------*/

//Keyboard support function
appfn_ptr keyb_task = keyb_mainmenu;


/*----------------------------------------------------------*/
