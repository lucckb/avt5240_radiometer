#ifndef EVENTS_H_
#define EVENTS_H_

/*----------------------------------------------------------*/

#include <stdint.h>

/*----------------------------------------------------------*/
//System Hz
#define HZ 100

//Number of timers
#define GENERIC_TIMERS 4

//Define timer assignement
#define DISPLAY_TIMER 0
#define DOSE_TIMER 1
#define BATTERY_TIMER 2
#define MAXRAD_TIMER 3

/*----------------------------------------------------------*/
//System timer set value
void timer_set(int id,int tick);

/*----------------------------------------------------------*/
//System timer get value
int timer_get(int id);

/*----------------------------------------------------------*/

//Configure system timer to requied interval
void systick_setup(int reload);

/*----------------------------------------------------------*/

//Klawisz powtorzony
#define KEYB_RPT 0x80

//Next key
#define KEY_NEXT  2
//Enter key
#define KEY_ENTER 1
//Escape key
#define KEY_ESC 4

/*----------------------------------------------------------*/

//Get kbd value
uint8_t keyb_get(void);

/*----------------------------------------------------------*/

//Keyboyard setup
void keyb_setup(void);


/*----------------------------------------------------------*/

#endif /*TIMER_H_*/
