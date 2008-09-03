#ifndef SYSTEM_H_
#define SYSTEM_H_


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


#endif /*SYSTEM_H_*/
