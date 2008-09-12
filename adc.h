#ifndef ADC_H_
#define ADC_H_

/*----------------------------------------------------------*/
//Configure ADC
void adc_setup(void);

/*----------------------------------------------------------*/

//Start adc conversion by software
void adc_startconv(void);


/*----------------------------------------------------------*/

//Get ADC value from selected channel
int adc_getval(void);

/*----------------------------------------------------------*/


#endif /*ADC_H_*/
