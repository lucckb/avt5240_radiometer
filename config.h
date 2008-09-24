/*
 * config.h
 *
 *  Created on: 2008-09-23
 *      Author: lucck
 */

#ifndef CONFIG_H_
#define CONFIG_H_

/*----------------------------------------------------------*/

//Return true if configuration is valid
bool isvalid_config(void);


/*----------------------------------------------------------*/

//Read configuration
void read_config(appState *app);


/*----------------------------------------------------------*/

//Save configuration unit and radiationa algoritm
void settings_write_config(appState *app);


/*----------------------------------------------------------*/

//Save maximum values
void maximum_write_config(appState *app);

/*----------------------------------------------------------*/
//Default time_t
#define DEFAULT_TIME_T 1222272738


/*----------------------------------------------------------*/



#endif /* CONFIG_H_ */
