#ifndef KEYB_H_
#define KEYB_H_

#include <stdint.h>

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

//This field is called from interrupt content
void on_keyb_timer_event(void);

/*----------------------------------------------------------*/

#endif /*KEYB_H_*/
