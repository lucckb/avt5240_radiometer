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

	int rCurrent = get_radiation(radiationCURRENT);
	int rLast = get_radiation(radiationLAST);

	//First line measured radiation
	lcd_setpos(1,1);
	if(app->unit==unitCLASSIC) lcd_printf("%duR",rCurrent);
	else lcd_printf("%d.%duS",rCurrent/100,rCurrent%100);

	lcd_setpos(1,2);
	switch(app->line)
	{
	//Curent counting mode in classic algoritm
	case lineRADIATION:
		if(app->unit==unitCLASSIC)
			lcd_printf("%duR",rLast);
		else
			lcd_printf("%d.%duS",rLast/100,rLast%100);
	break;
	//Get actual dose
	case lineDOSE:
		if(app->unit==unitCLASSIC)
			lcd_printf("%d.%dmr",(app->dose*6)/100,((app->dose*6)%100+5)/10);
		else
			lcd_printf("%d.%duS",(app->dose*6)/10000,((app->dose*6)%10000+5)/10);
	break;
	//Radiation description
	case lineMAX:
		if(app->unit==unitCLASSIC)
			lcd_printf("%duR",app->radiationMax);
		else
			lcd_printf("%d.%duS",app->radiationMax/100,app->radiationMax%100);
	break;
	}
}


/*----------------------------------------------------------*/
//Display function pointers
const appfn_ptr disp_funcs[] =
{
		display_radiation,		//Display current radiation
};

/*----------------------------------------------------------*/
