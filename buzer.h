#ifndef BUZER_H_
#define BUZER_H_


/*----------------------------------------------------------*/

//This function is called from interrupt handler
void on_buzzer_timer_event(void);

/*----------------------------------------------------------*/

//Setup buzzer to selected rate
void buzer_alarm(bool onOff);

/*----------------------------------------------------------*/
//Generate buzer click
static inline void buzer_click(void)
{
	//buzer_control(buzerPulse);
	GPIOA->ODR ^= (1<<7);
}

/*----------------------------------------------------------*/

//Enable 20kHZ signal voltage dubler for LCD power supply
void lcd_pwm_setup(void);


/*----------------------------------------------------------*/

#endif /*BUZER_H_*/
