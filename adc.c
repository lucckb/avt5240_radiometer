#include "stm32f10x_lib.h"
#include "adc.h"

/*----------------------------------------------------------*/

/* SQR1 register Mask */
#define SQR1_CLEAR_Mask             ((u32)0xFF0FFFFF)
/* CR2 register Mask */
#define CR2_CLEAR_Mask              ((u32)0xFFF1F7FD)
/* CR1 register Mask */
#define CR1_CLEAR_Mask              ((u32)0xFFF0FEFF)
/* SQR Channel mask */
#define SQR_CH_Mask  0x1f
/* ADC ADON mask */
#define CR2_ADON_Set                ((u32)0x00000001)
#define CR2_ADON_Reset              ((u32)0xFFFFFFFE)
/* ADC RSTCAL mask */
#define CR2_RSTCAL_Set              ((u32)0x00000008)
/* ADC CAL mask */
#define CR2_CAL_Set                 ((u32)0x00000004)
/* ADC Software start mask */
#define CR2_EXTTRIG_SWSTART_Set     ((u32)0x00500000)
#define CR2_EXTTRIG_SWSTART_Reset   ((u32)0xFFAFFFFF)
#define GPIO_CRL_PA5_MASK (0xf<<20)

/*----------------------------------------------------------*/
//Configure ADC
void adc_setup(void)
{
  
  //Enable analog ADC 
  RCC->APB2ENR |= RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOA;
	
  //Configure GPIO PA5 as Analog Input
  GPIOA->CRL &= ~GPIO_CRL_PA5_MASK;
  
  
  // Clear DUALMOD and SCAN bits 
  ADC1->CR1 &= CR1_CLEAR_Mask;
  
  //Adc work in independent mode
  ADC1->CR1 |= ADC_Mode_Independent ;
  
  // Clear CONT, ALIGN and EXTSEL bits 
  ADC1->CR2 &= CR2_CLEAR_Mask;
  
  // Disable external triger mode
  ADC1->CR2 |=  ADC_ExternalTrigConv_None;
  
  // Clear L bits 
  ADC1->SQR1 &= SQR1_CLEAR_Mask;
  
  // Write to ADCx SQR1 numer of channels in conversion 
  ADC1->SQR1 |= (0<<20);
  
  //Configure sample timing to 239 on CH5
  ADC1->SMPR2 &= ~ADC_SampleTime_239Cycles5 << 15;
  ADC1->SMPR2 |= ADC_SampleTime_239Cycles5 << 15;
  
  //Select CH5 for first seqence
  ADC1->SQR3 &= SQR_CH_Mask;
  ADC1->SQR3 |= 5;
  
  //Enable ADC
  ADC1->CR2 |= CR2_ADON_Set;
  
  // *Calibration ADC*
  //Initialize calibration register
  ADC1->CR2 |= CR2_RSTCAL_Set;
  //Wait for initialization
  while (ADC1->CR2 & CR2_RSTCAL_Set);
  //ADC start calibration
  ADC1->CR2 |= CR2_CAL_Set;
  //Wait for calibration
  while(ADC1->CR2 & CR2_CAL_Set);
}

/*----------------------------------------------------------*/
//Start adc conversion by software
void adc_startconv(void)
{
	ADC1->CR2 |= CR2_EXTTRIG_SWSTART_Set;
}

/*----------------------------------------------------------*/
//Get ADC value from selected channel
int adc_getval(void)
{
	return (ADC1->SR & ADC_FLAG_EOC)?(ADC1->DR):(-1);
}
/*----------------------------------------------------------*/


