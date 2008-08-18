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


#include "stm32f10x_lib.h"
#include "lcd.h"
#include <stdint.h>
#include <stdbool.h>
#include "stm32f10x_lib.h"

GPIO_InitTypeDef GPIO_InitStructure;

/*----------------------------------------------------------*/
//Definicje poszczegolnych bitow
static inline void RS(int n)
{
    if(n) GPIO_SetBits(GPIOC,GPIO_Pin_12);
    else GPIO_ResetBits(GPIOC,GPIO_Pin_12);
}

static inline void RW(int n)
{
    if(n) GPIO_SetBits(GPIOC,GPIO_Pin_11);
    else GPIO_ResetBits(GPIOC,GPIO_Pin_11);
}

static inline void EN(int n)
{
    if(n) GPIO_SetBits(GPIOC,GPIO_Pin_10);
    else GPIO_ResetBits(GPIOC,GPIO_Pin_10);
}



static inline void wr4(int n)
{

    if(n & 0x80) GPIO_SetBits(GPIOC,GPIO_Pin_0);
    else GPIO_ResetBits(GPIOC,GPIO_Pin_0);
    
    if(n & 0x40) GPIO_SetBits(GPIOC,GPIO_Pin_1);
    else GPIO_ResetBits(GPIOC,GPIO_Pin_1);

    if(n & 0x20) GPIO_SetBits(GPIOC,GPIO_Pin_2);
    else GPIO_ResetBits(GPIOC,GPIO_Pin_2);
    
    if(n & 0x10) GPIO_SetBits(GPIOC,GPIO_Pin_3);
    else GPIO_ResetBits(GPIOC,GPIO_Pin_3);

}

static inline uint8_t rd4(void)
{
    int r = 0;
    int n = GPIO_ReadInputData(GPIOC);
    if(n & 1) r|=0x80;
    if(n & 2) r|=0x40;
    if(n & 4) r|=0x20;
    if(n & 8) r|=0x10;
    return r;
}


#define DPORT_IN false
#define DPORT_OUT true

//Przelacza port IO z wejscia na wyjscie i odwrotnie
static inline void dportDir(bool out)
{
    if(out)
    {
       GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 |GPIO_Pin_2| GPIO_Pin_3;
       GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
       GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
       GPIO_Init(GPIOC, &GPIO_InitStructure);
    }
    else
    {
       GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 |GPIO_Pin_2| GPIO_Pin_3;
       GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
       GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
       GPIO_Init(GPIOC, &GPIO_InitStructure);
    }
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
    //delay_us(2000);
    //return;
    uint8_t r;
	int tout = 16000;
	//Sprawdzenie czy jest wolny wyswietlacz
	do
	{
		EN(0);
		RS(0);
		RW(1);
		dportDir(DPORT_IN);
		EN(1);
		delay_us(1);
        r = rd4();
		EN(0);
        delay_us(2);
        EN(1);
        delay_us(1);
        EN(0);
		if(--tout == 0) return;
	}
	while(r & 0x80);
	dportDir(DPORT_OUT);
}

/*----------------------------------------------------------*/

//Wysyla dana lub instrukcje w zaleznosci od id
static void write_lcd(uint8_t val,bool command)
{
    //Wlasciwy zapis danej do portu (Interfejs 4 bitowy)
	EN(0);
	RW(0);
	if(command) RS(0);
    else RS(1);
    wr4(val);
	EN(1);
	delay_us(1);
    EN(0);
    delay_us(1);
    wr4(val<<4);
    EN(1);
    delay_us(1);
    EN(0);
}


/*----------------------------------------------------------*/
//Inicjalizacja modulu LCD
void lcdInit(void)
{
    /** Inicjalizacja  GPIO **/
    /* Enable GPIOC clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    /* Configure PC.4 as Output push-pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 |GPIO_Pin_2| GPIO_Pin_3 |GPIO_Pin_10 |GPIO_Pin_11 |GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    EN(0); RW(0); EN(0);

    wr4(0x30);
    EN(1);
    delay_us(1);
    EN(0);
    delay_us(20000);
    EN(1);
    delay_us(1);
    EN(0);
    delay_us(20000);
    EN(1);
    delay_us(1);
    EN(0);
    wr4(0x20);
    delay_us(20000);
    EN(1);
    delay_us(1);
    EN(0);
    delay_us(20000);


    write_lcd(0x28,1);		//Interfejs 4 bitowy 2 linie
    wait4lcd();
	write_lcd(0x6,1);		//Tryb pracy
    wait4lcd();
	write_lcd(0x0c,1);		//Wlacz wyswietlacz
    wait4lcd();
	write_lcd(1,1);	//Wyczysc wyswietlacz..
    wait4lcd();


/*
     write_lcd(0x28, 1);
     wait4lcd();
	//Wyczysc wyswietlacz
     write_lcd(0x01, 1);
     wait4lcd();
	//Ustawienie kierunku zwiekszania linii
     write_lcd(0x06, 1);
     wait4lcd();
	//Przesuwanie kursora w prawo
     write_lcd(0x0c, 1);
     wait4lcd();
	//Ustawienie kursora na pozycji poczatkowej
     write_lcd(0x02, 1);
     wait4lcd();
    //Ustawienie adresu znakow na poczatek
     write_lcd(0x80, 1);
     wait4lcd();
*/

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


