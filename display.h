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
	int radiationMax;
	//Radiation max timestamp
	time_t radiationMaxTime;
	//Power voltage in ten mV
	int Vpwr;
	//Define cursor position in scrolling
	uint8_t scrollPos;

} appState;

/*----------------------------------------------------------*/

//Typedef to function pointer
typedef void (*appfn_ptr)(appState *);

/*----------------------------------------------------------*/

//Global display functions
extern const appfn_ptr disp_funcs[];

/*----------------------------------------------------------*/

//Keyboyard support function
extern appfn_ptr keyb_task;

/*----------------------------------------------------------*/
#endif /* DISPLAY_H_ */
