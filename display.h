/*
 * display.h
 *
 *  Created on: 2008-09-20
 *      Author: lucck
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_

/*----------------------------------------------------------*/
#include "radiation.h"
#include "rtc.h"
/*----------------------------------------------------------*/

//Display refresh interval
#define DISPLAY_REFRESH HZ/5

/*----------------------------------------------------------*/

//Enumeration for menu items
enum mnuItems
{
	mnuRADIATION,		//Display radiation
	mnuMAX_RADIATION,	//Maximum radiation
	mnuDISP_DOSE,		//Display dose
	mnuDISP_DATE,		//Display date
	mnuDISP_CONF,		//Display conf
    mnuSET_ALARM,		//Set radiation alarm
    mnuSET_UNIT,		//Display unit config
    mnuSET_ALGO,		//Display algo config
};


/*----------------------------------------------------------*/
//Enumeration for unit
enum unitConfig
{
	unitCLASSIC,		//Classic unit (Rontgen per hour)
	unitSI				//SI Unit (Sivert per hour)
};


/*----------------------------------------------------------*/
//Global application structure
typedef struct appState
{
	//Current displayed menu
	enum mnuItems menu;
	//Current algo
	enum radiationCountMode radiationAlgo;
	//Unit configuration
	enum unitConfig unit;
	//Radiation dose
	unsigned int dose;
	//Radiation max
	unsigned int radiationMax;
	//Radiation max timestamp
	time_t radiationMaxTime;

} appState;

/*----------------------------------------------------------*/

//Typedef to function pointer
typedef void (*appfn_ptr)(appState *);

/*----------------------------------------------------------*/

//Global display functions
extern const appfn_ptr disp_funcs[];

/*----------------------------------------------------------*/
#endif /* DISPLAY_H_ */
