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

//Initialize LCD
void lcd_init(void);

/*----------------------------------------------------*/

//Wyczysc lcd
void lcd_clear(void);

/*----------------------------------------------------*/
//Shift display to right
#define LCD_SHIFT_RIGHT 0x1A

//Shift display to left
#define LCD_SHIFT_LEFT 0x18

//Cursor to home position
#define LCD_CURSOR_HOME 0x02

//Show cursor
#define LCD_CURSOR_SHOW 0x0F

//Cursor hide
#define LCD_CURSOR_HIDE 0x0C

/*----------------------------------------------------*/
//Wyswietl znak
void lcd_putch(char ch);

/*----------------------------------------------------*/
//LCD put space
void lcd_putspace(uint8_t n);

/*----------------------------------------------------*/
//Send command

void lcd_command(uint8_t cmd);


/*----------------------------------------------------*/

//Set position
void lcd_setpos(uint8_t x,uint8_t y);

/*----------------------------------------------------*/

//LCD printf
int lcd_printf(const char *format, ...);

/*----------------------------------------------------*/


#endif

