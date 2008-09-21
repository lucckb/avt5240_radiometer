#ifndef EVENTS_H_
#define EVENTS_H_

/*----------------------------------------------------------*/

#include <stdint.h>

/*----------------------------------------------------------*/
//System Hz
#define HZ 100

//Number of timers
#define GENERIC_TIMERS 3

//Define timer assignement
#define DISPLAY_TIMER 0
#define DOSE_TIMER 1
#define BATTERY_TIMER 2

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

/*----------------------------------------------------------*/
//Last key pressed variable

extern volatile uint8_t keyb_key;

/*----------------------------------------------------------*/

//Keyboyard setup
void keyb_setup(void);


/*----------------------------------------------------------*/

//Get pressed key
static inline uint8_t keyb_get(void) { return keyb_key; }

/*----------------------------------------------------------*/

#endif /*TIMER_H_*/
