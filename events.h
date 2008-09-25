#ifndef EVENTS_H_
#define EVENTS_H_

/*----------------------------------------------------------*/

#include <stdint.h>
#include "system.h"

/*----------------------------------------------------------*/
//System Hz
#define HZ 100


/*----------------------------------------------------------*/

//System timer interrupt in microsecond interval
void systick_setup(int reload);

/*----------------------------------------------------------*/

//Jiffiess typedef
typedef unsigned long timer_t;


/*----------------------------------------------------------*/

//Global jiffies
extern volatile timer_t jiffies;


/*----------------------------------------------------------*/
//Timer start macro
#define timer_start(t_start) (t_start) = jiffies;

/*----------------------------------------------------------*/

//Timer elapsed macro
#define timer_elapsed(t_start,timeout) jiffies-(t_start)>=(timeout)

/*----------------------------------------------------------*/
//Function waiting for timer
static inline void timer_wait(timer_t wait_time)
{
	timer_t t0 = jiffies;
	while(jiffies-t0<wait_time)
	{
		iwdt_reset();
		wfi();
	}
}

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
