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
#include <stdarg.h>


/*----------------------------------------------------------*/
#define LCD_RS_PIN (1<<9)	//LCD RS signal Pin9
#define LCD_RW_PIN (1<<10)  //LCD RW signal Pin10
#define LCD_EN_PIN (1<<11)  //LCD RW signal Pin11
#define LCD_PORT GPIOB		//LCD GPIO port
#define LCD_APB_CLOCK_BIT RCC_APB2Periph_GPIOB //LCD APB clock bit

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
    RCC->APB2ENR |= LCD_APB_CLOCK_BIT;

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
void lcd_init(void)
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
//Wyczysc lcd
void lcd_clear(void)
{
    //Wyslij komende czyszczenia
    write_lcd(1,1);
    //Czekaj
    wait4lcd();
}

/*----------------------------------------------------------*/
//Wrzuca spacje
void lcd_putspace(uint8_t n)
{
	for(int i=0;i<n;i++)
		lcd_putch(' ');
}

/*----------------------------------------------------------*/
//Wyswietl znak
void lcd_putch(char ch)
{
    //Wyslij literke
    write_lcd(ch,0);
    //Czekaj
    wait4lcd();
}


/*----------------------------------------------------------*/
//Wyslij rozkaz
void lcd_command(uint8_t cmd)
{
    write_lcd(cmd,1);
    wait4lcd();
}

/*----------------------------------------------------------*/
//Ustaw pozycje
void lcd_setpos(uint8_t x,uint8_t y)
{
	x-=1;
	if(y==2) x |= 0x40;
    write_lcd(0x80 | x,1);
    wait4lcd();
}

/*----------------------------------------------------------*/
// PRINTF IMPLEMENTATION
/*----------------------------------------------------------*/
static void printchar(char **str, int c)
{
	if (str) {
		**str = c;
		++(*str);
	}
	else {
		lcd_putch(c);
	}
}

#define PAD_RIGHT 1
#define PAD_ZERO 2

static int prints(char **out, const char *string, int width, int pad)
{
	register int pc = 0, padchar = ' ';

	if (width > 0) {
		register int len = 0;
		register const char *ptr;
		for (ptr = string; *ptr; ++ptr) ++len;
		if (len >= width) width = 0;
		else width -= len;
		if (pad & PAD_ZERO) padchar = '0';
	}
	if (!(pad & PAD_RIGHT)) {
		for ( ; width > 0; --width) {
			printchar (out, padchar);
			++pc;
		}
	}
	for ( ; *string ; ++string) {
		printchar (out, *string);
		++pc;
	}
	for ( ; width > 0; --width) {
		printchar (out, padchar);
		++pc;
	}

	return pc;
}

/* the following should be enough for 32 bit int */
#define PRINT_BUF_LEN 12

static int printi(char **out, int i, int b, int sg, int width, int pad, int letbase)
{
	char print_buf[PRINT_BUF_LEN];
	register char *s;
	register int t, neg = 0, pc = 0;
	register unsigned int u = i;

	if (i == 0) {
		print_buf[0] = '0';
		print_buf[1] = '\0';
		return prints (out, print_buf, width, pad);
	}

	if (sg && b == 10 && i < 0) {
		neg = 1;
		u = -i;
	}

	s = print_buf + PRINT_BUF_LEN-1;
	*s = '\0';

	while (u) {
		t = u % b;
		if( t >= 10 )
			t += letbase - '0' - 10;
		*--s = t + '0';
		u /= b;
	}

	if (neg) {
		if( width && (pad & PAD_ZERO) ) {
			printchar (out, '-');
			++pc;
			--width;
		}
		else {
			*--s = '-';
		}
	}

	return pc + prints (out, s, width, pad);
}

static int print(char **out, const char *format, va_list args )
{
	register int width, pad;
	register int pc = 0;
	char scr[2];

	for (; *format != 0; ++format) {
		if (*format == '%') {
			++format;
			width = pad = 0;
			if (*format == '\0') break;
			if (*format == '%') goto out;
			if (*format == '-') {
				++format;
				pad = PAD_RIGHT;
			}
			while (*format == '0') {
				++format;
				pad |= PAD_ZERO;
			}
			for ( ; *format >= '0' && *format <= '9'; ++format) {
				width *= 10;
				width += *format - '0';
			}
			if( *format == 's' ) {
				register char *s = (char *)va_arg( args, int );
				pc += prints (out, s?s:"(null)", width, pad);
				continue;
			}
			if( *format == 'd' ) {
				pc += printi (out, va_arg( args, int ), 10, 1, width, pad, 'a');
				continue;
			}
			if( *format == 'x' ) {
				pc += printi (out, va_arg( args, int ), 16, 0, width, pad, 'a');
				continue;
			}
			if( *format == 'X' ) {
				pc += printi (out, va_arg( args, int ), 16, 0, width, pad, 'A');
				continue;
			}
			if( *format == 'u' ) {
				pc += printi (out, va_arg( args, int ), 10, 0, width, pad, 'a');
				continue;
			}
			if( *format == 'c' ) {
				/* char are converted to int then pushed on the stack */
				scr[0] = (char)va_arg( args, int );
				scr[1] = '\0';
				pc += prints (out, scr, width, pad);
				continue;
			}
		}
		else {
		out:
			printchar (out, *format);
			++pc;
		}
	}
	if (out) **out = '\0';
	va_end( args );
	return pc;
}

int lcd_printf(const char *format, ...)
{
        va_list args;

        va_start( args, format );
        return print( 0, format, args );
}

