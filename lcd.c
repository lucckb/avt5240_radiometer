/*
    LCD Char STM32 library
    Copyright (C) 2008  Lucjan Bryndza <lucjan.bryndza@ep.com.pl>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "stm32f10x_lib.h"
#include "lcd.h"
#include <stdint.h>
#include <stdbool.h>


/*----------------------------------------------------------*/
#define LCD_RS_PIN (1<<9)	//LCD RS signal Pin9
#define LCD_RW_PIN (1<<10)  //LCD RW signal Pin10
#define LCD_EN_PIN (1<<11)  //LCD RW signal Pin11
#define LCD_PORT GPIOB		//LCD GPIO port

#define LCD_DATA_BITS_MASK 0xf000
#define LCD_GPIO_DATA_CRH_MASK 0xffff0000
#define LCD_GPIO_ALL_CRH_MASK 0xfffffff0
#define GPIO_MODE_PULLUP_INPUT 2
#define GPIO_MODE_10MHZ 1


/*----------------------------------------------------------*/
//LCD RW signal control
static inline void rs(bool bit_val)
{
	if(bit_val) LCD_PORT->BSRR = LCD_RS_PIN;
	else LCD_PORT->BRR = LCD_RS_PIN;
}

/*----------------------------------------------------------*/
//LCD RW signal control
static inline void rw(bool bit_val)
{
	if(bit_val) LCD_PORT->BSRR = LCD_RW_PIN;
	else  LCD_PORT->BRR = LCD_RW_PIN;
}

/*----------------------------------------------------------*/
//LCD E signal control
static inline void en(bool bit_val)
{
	if(bit_val) LCD_PORT->BSRR = LCD_EN_PIN;
	else  LCD_PORT->BRR = LCD_EN_PIN;
}


/*----------------------------------------------------------*/
//Write data to LCD bus
static inline void wr_data(uint8_t d_7_4_val)
{
	LCD_PORT->BRR = LCD_DATA_BITS_MASK;
	LCD_PORT->BSRR = ((int)d_7_4_val & 0xf0) << 8;
}


/*----------------------------------------------------------*/
//Read data from LCD bus 
static inline uint8_t rd_data(void)
{
	return ((LCD_PORT->IDR>>12)<<4)& 0xf0;
}

/*----------------------------------------------------------*/
#define DPORT_IN false	/* Input mode */
#define DPORT_OUT true  /* Output mode */

//Switch LCD Bus from input to output
static inline void dataport_dir(bool out)
{
    if(out)
    {

       //Output mode 10MHz bit15-bit12
       LCD_PORT->CRH &= ~LCD_GPIO_DATA_CRH_MASK;
       LCD_PORT->CRH |= (GPIO_MODE_10MHZ << 28) | (GPIO_MODE_10MHZ << 24) |
       				(GPIO_MODE_10MHZ << 20) | (GPIO_MODE_10MHZ << 16);
       
    }
    else
    {
      //Input mode pull up bit15-bit12
       LCD_PORT->CRH &= ~LCD_GPIO_DATA_CRH_MASK;
       LCD_PORT->CRH |= (GPIO_MODE_PULLUP_INPUT << 30) | (GPIO_MODE_PULLUP_INPUT << 26) |
       					(GPIO_MODE_PULLUP_INPUT << 22) | (GPIO_MODE_PULLUP_INPUT << 18);
       //Pull up configuration
       LCD_PORT->ODR |= LCD_DATA_BITS_MASK; 
    }
}

/*----------------------------------------------------------*/
/** GPIO Initialize 
 * Initialize bit 15-9 in LCD port 
 **/
static void gpio_init(void)
{

	/* Enable GPIOC clock */
    RCC->APB2ENR |= RCC_APB2Periph_GPIOC;
    
    //Output mode 10MHz bit15-bit12
    LCD_PORT->CRH &= ~LCD_GPIO_ALL_CRH_MASK;
    LCD_PORT->CRH |= (GPIO_MODE_10MHZ << 28) | (GPIO_MODE_10MHZ << 24) |
           			  (GPIO_MODE_10MHZ << 20) | (GPIO_MODE_10MHZ << 16) |
           			  (GPIO_MODE_10MHZ << 12) | (GPIO_MODE_10MHZ << 8) | 
           			  (GPIO_MODE_10MHZ << 4);
 
}

/*----------------------------------------------------------*/
//czeka n mikrosekund
static void delay_us(int us)
{
    us *= 2;
    while(us--)
	asm volatile ("nop");
}

/*----------------------------------------------------------*/
//Czeka sprawdzajac flage zajetosci
static void wait4lcd(void)
{
    uint8_t r;
	int tout = 16000;
	//Sprawdzenie czy jest wolny wyswietlacz
	do
	{
		en(0);
		rs(0);
		rw(1);
		dataport_dir(DPORT_IN);
		en(1);
		delay_us(1);
        r = rd_data();
		en(0);
        delay_us(2);
        en(1);
        delay_us(1);
        en(0);
		if(--tout == 0) return;
	}
	while(r & 0x80);
	dataport_dir(DPORT_OUT);
}

/*----------------------------------------------------------*/

//Wysyla dana lub instrukcje w zaleznosci od id
static void write_lcd(uint8_t val,bool command)
{
    //Wlasciwy zapis danej do portu (Interfejs 4 bitowy)
	en(0);
	rw(0);
	if(command) rs(0);
    else rs(1);
    wr_data(val);
	en(1);
	delay_us(1);
    en(0);
    delay_us(1);
    wr_data(val<<4);
    en(1);
    delay_us(1);
    en(0);
}


/*----------------------------------------------------------*/
//Inicjalizacja modulu LCD
void lcdInit(void)
{
    
	//Initialize gpio
	gpio_init();
	
    en(0); rw(0); en(0);

    wr_data(0x30);
    en(1);
    delay_us(1);
    en(0);
    delay_us(20000);
    en(1);
    delay_us(1);
    en(0);
    delay_us(20000);
    en(1);
    delay_us(1);
    en(0);
    wr_data(0x20);
    delay_us(20000);
    en(1);
    delay_us(1);
    en(0);
    delay_us(20000);


    write_lcd(0x28,1);		//Interfejs 4 bitowy 2 linie
    wait4lcd();
	write_lcd(0x6,1);		//Tryb pracy
    wait4lcd();
	write_lcd(0x0c,1);		//Wlacz wyswietlacz
    wait4lcd();
	write_lcd(1,1);	//Wyczysc wyswietlacz..
    wait4lcd();

}

/*----------------------------------------------------------*/
//Put string
void lcdPutStr(char *nap)
{
	while(*nap)
	{
		//Wyswietl znak
        write_lcd(*nap,0);
        //Czekaj
        wait4lcd();
	    //Przejdz do nastepnego znaku
        nap++;
	}
}

/*----------------------------------------------------------*/
//Wyczysc lcd
void lcdClear(void)
{
    //Wyslij komende czyszczenia
    write_lcd(1,1);
    //Czekaj
    wait4lcd();
}

/*----------------------------------------------------------*/
//Wyswietl znak
void lcdPutChar(char ch)
{
    //Wyslij literke
    write_lcd(ch,0);
    //Czekaj
    wait4lcd();
}

/*----------------------------------------------------------*/
//Wyswietl liczbe
void lcdPutInt(unsigned int num)
{
    char buf[10];
    unsigned int calk;
    int i=0;
    calk = num;
    while(calk)
    {
      calk /= 10;
      i++;
    }
    if(i)
    {
        calk = num;
        buf[i] = 0;
        for(--i;i>=0;i--)
        {
           buf[i] = calk % 10 + '0';
           calk /= 10;
        }
    }
    else
    {
        buf[0] = '0';
        buf[1] = 0;
    }
	char *str = buf;
	while(*str)
	{
        write_lcd(*str,0);
        wait4lcd();
        str++;
    }
}

/*----------------------------------------------------------*/
//Wyslij rozkaz
void lcdCommand(uint8_t disp,uint8_t blink,uint8_t cursor)
{
    write_lcd(disp+blink+cursor+0x08,1);
    wait4lcd();
}

/*----------------------------------------------------------*/
//Ustaw pozycje
void lcdSetPos(uint8_t pos)
{
    write_lcd(0x80 | pos,1);
    wait4lcd();
}


