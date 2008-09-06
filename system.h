#ifndef SYSTEM_H_
#define SYSTEM_H_


/*----------------------------------------------------------*/
//System Hz 
#define HZ 100

/*----------------------------------------------------------*/
//Cortex stm32 System setup
void system_setup(void);

/*----------------------------------------------------------*/

//Configure system timer to requied interval
void systick_setup(int reload);

/*----------------------------------------------------------*/

//Enable 20kHZ signal voltage dubler for LCD power supply
void lcd_pwm_setup(void);

/*----------------------------------------------------------*/
//Internal use only definition
extern volatile int sysTimer;

/*----------------------------------------------------------*/
//System timer set value 
static inline void systick_set(int tick) { sysTimer = tick; }

/*----------------------------------------------------------*/
//System timer get value
static inline int systick_get(void) { return sysTimer; } 

/*----------------------------------------------------------*/
//Wait for selected time
static inline void systick_wait(int tick) { sysTimer = tick; while(sysTimer); }

/*----------------------------------------------------------*/
#endif /*SYSTEM_H_*/
