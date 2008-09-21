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


/*----------------------------------------------------------*/

//User interface task
static void display_radiation(appState *app)
{
	static bool dispBat = false;
	//Get radiation
	int rad = get_radiation(radiationCURRENT);

	if(app->radiationAlgo!= radiationCountSTD)
	{
		//Alghoritm 1/n
		if(app->unit==unitCLASSIC)
		{
			lcd_setpos(1,1);
			if(rad) lcd_printf("%d       ",rad);
			else lcd_printf("----    ");
			lcd_setpos(1,2);
			lcd_printf("uR/h");
		}
		else
		{
			lcd_setpos(1,1);
			if(rad) lcd_printf("%d.%02d    ",rad/100,rad%100);
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
			lcd_printf("%duRh    ",rad);
		else
			lcd_printf("%d.%duSh  ",rad);

		lcd_setpos(1,2);
		lcd_printf("%duRh    ",get_radiation(radiationLAST));
	}
}


/*----------------------------------------------------------*/
//Display function pointers
const appfn_ptr disp_funcs[] =
{
		display_radiation,		//Display current radiation
};

/*----------------------------------------------------------*/
