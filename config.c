/*
 * config.c
 *
 *  Created on: 2008-09-23
 *      Author: lucck
 */

#include <stdbool.h>
#include <stdint.h>
#include "system.h"
#include "rtc.h"
#include "display.h"

/*----------------------------------------------------------*/

//Magic key
#define CFG_MAGIC_KEY 0x197A
//Magic bkp addr
#define CFG_MAGIC_ADDR 10
//Configuration address
#define CFG_CONFIG_ADDR 9

//Radiation backup addr
#define CFG_RADVAL_ADDR 1
#define CFG_RADTIME_ADDR 3


/*----------------------------------------------------------*/

//Return true if configuration is valid
bool isvalid_config(void)
{
	//Compare with validate mask
	return rtc_bkp_read(CFG_MAGIC_ADDR)==CFG_MAGIC_KEY;
}

/*----------------------------------------------------------*/
//Read configuration
void read_config(appState *app)
{
	//If configuration is valid read it
	if(isvalid_config()==false)
	{
		//Save on lower byte config algo and on hi byte radiation algo
		rtc_bkp_write(CFG_CONFIG_ADDR,(uint16_t)unitSI<<8|radiationCountMEDIUM);
		//Zero dose
		rtc_bkp_write(CFG_RADTIME_ADDR,0);
		rtc_bkp_write(CFG_RADTIME_ADDR+1,0);
		rtc_bkp_write(CFG_RADVAL_ADDR,0);
		rtc_bkp_write(CFG_RADVAL_ADDR+1,0);
		//Write magic key
		rtc_bkp_write(CFG_MAGIC_ADDR,CFG_MAGIC_KEY);
	}

	uint16_t val;

	//Read config
	val = rtc_bkp_read(CFG_CONFIG_ADDR);
	app->unit = val >> 8;
	app->radiationAlgo = val & 0xff;

	//Read max radiation value
	uint16_t *valp;
	valp = (uint16_t*)&app->radiationMax;
	*valp = rtc_bkp_read(CFG_RADVAL_ADDR);
	*(valp+1) = rtc_bkp_read(CFG_RADVAL_ADDR+1);

	//Read max radiation time
	valp = (uint16_t*)&app->radiationMaxTime;
	*valp = rtc_bkp_read(CFG_RADTIME_ADDR);
	*(valp+1) = rtc_bkp_read(CFG_RADTIME_ADDR+1);

}
/*----------------------------------------------------------*/
//Save configuration unit and radiationa algoritm
void settings_write_config(appState *app)
{
	//Save on lower byte config algo and on hi byte radiation algo
	rtc_bkp_write(CFG_CONFIG_ADDR,(uint16_t)app->unit<<8 | app->radiationAlgo);
}

/*----------------------------------------------------------*/
//Save maximum values
void maximum_write_config(appState *app)
{
	//Read max radiation value
	uint16_t *valp;
	valp = (uint16_t*)&app->radiationMax;
	rtc_bkp_write(CFG_RADVAL_ADDR,*valp);
	rtc_bkp_write(CFG_RADVAL_ADDR+1,*(valp+1));

	//Read max radiation time
	valp = (uint16_t*)&app->radiationMaxTime;
    rtc_bkp_write(CFG_RADTIME_ADDR,*valp);
    rtc_bkp_write(CFG_RADTIME_ADDR+1,*(valp+1));
}

/*----------------------------------------------------------*/
