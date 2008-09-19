/*
    Str912 book example8 - LCD Char C library
    Copyright (C) 2007  Lucjan Bryndza <lucjan.bryndza@ep.com.pl>

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


//Definicja pinow
#ifndef __LCD_H__
#define __LCD_H__

#include <stdint.h>

/*----------------------------------------------------*/

void lcd_init(void);

/*----------------------------------------------------*/

void lcd_puts(char *nap);

/*----------------------------------------------------*/

//Wyczysc lcd
void lcd_clear(void);

/*----------------------------------------------------*/

//Wyswietl znak
void lcd_putch(char ch);

/*----------------------------------------------------*/
//Send command
#define LCD_BLINK     0x01    // Alias for blinking cursor
#define LCD_NOBLINK   0x00    // Alias for non blinking cursor
#define LCD_SHOW      0x02    // Alias for cursor on
#define LCD_HIDE      0x00    // Alias for cursor off
#define LCD_ON        0x04    // Alias for display on
#define LCD_OFF       0x00    // Alias for display off

void lcd_command(uint8_t disp,uint8_t blink,uint8_t cursor);


/*----------------------------------------------------*/

//Set position
void lcd_setpos(uint8_t x,uint8_t y);

/*----------------------------------------------------*/

//LCD printf
int lcd_printf(const char *format, ...);

/*----------------------------------------------------*/


#endif

