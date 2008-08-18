/*
    Str912 book example8 - Simple LCD character display in C source
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


#include "91x_map.h"
#include "91x_scu.h"
#include "91x_fmi.h"
#include "91x_vic.h"
#include "91x_wiu.h"
#include "armint.h"
#include "lcd.h"
#include <stdint.h>

/*----------------------------------------------------------------------*/

// delay for n cycles
static void delay(int n)
{
    while(n--)
	asm volatile ("nop");
}

/*----------------------------------------------------------------------*/

/* Ustawienie parametrow zegarowych */
//Czestotliwosc 96MHz
#define PLL_N 192
#define PLL_M 25
#define PLL_P 2
#define SCU_PLLEN 0x80000

static void  setup_pll(void)
{
    //Jezeli petla jest zsynchronizowana tzn ze jest ok
    if(SCU->SYSSTATUS & SCU_FLAG_LOCK) return;
    //Konfiguracja uruchomienie i synchronizacja PLL
    SCU->PLLCONF = (PLL_P<<16) | (PLL_N<<8) | PLL_M;
    SCU->PLLCONF |= SCU_PLLEN;
    while(~SCU->SYSSTATUS & SCU_FLAG_LOCK);

    //Ustawiamy pamiec flash na dwa cykle oczekiwania (dla 96MHz)
    FMI->CR |= FMI_WRITE_WAIT_STATE_1;
    *(vu16*)FMI_BANK_1 = 0x60;
    *(vu16*)(FMI_BANK_1|FMI_READ_WAIT_STATE_2|FMI_LVD_ENABLE|FMI_PWD_ENABLE|FMI_FREQ_HIGH) = 0x03;

    //Ustawiamy F clk na koncu bedzie z PLL-a
    SCU->CLKCNTR = SCU_EMIBCLK_Div2 | SCU_USBCLK_MCLK2 | SCU_PCLK_Div2;
}


/*----------------------------------------------------------------------*/
/* Ustawienie portow IO */
static void setup_gpio(void)
{
    // Wylacz sygnal zerowania dla portu 7 i 3
    SCU->PRR1 |= __GPIO3 ;

    //Wlacz sygnal zegarowy dla portu 7 i 3
    SCU->PCGR1 |= __GPIO3;

    //Caly port 3 jako wejscie
    SCU->GPIOIN[3] = 0x00;

    //Port 3 jako wejsciowy
    GPIO3->DDR = 0;

}

/*----------------------------------------------------------------------*/
//Licznik przerwan
static volatile uint32_t irqCnt;

//Przerwanie niemaskowalne fiq
static void irqIntHandler(void) __attribute__ ((interrupt("IRQ")));

void irqIntHandler(void)
{
    
    //Przepisz kto zglosil do ledow
    GPIO7->DR[0xff<<2] = ++irqCnt;
    //Kasuj zrodlo przerwan
    WIU->PR = 0x80;
    //Informacja dla VIC (koniec przerwania)
    VIC1->VAR = 0;
}

/*----------------------------------------------------------------------*/
#define VIC1_WIU0 (1<<10)
#define VIC1_WIU0_BIT 10
#define VIC_SLOT_EN (1<<5)
#define WIU_INT_EN 0x02
#define WKUPSEL_EXTINT7 0x07

//Setup external interrupt
static void setup_extint(void)
{
    /*** Ustawienie WIU (WakeUP Interrupt Unit) ***/
    //Wylacz reset
    SCU->PRR1 |= __WIU;
    //Wlacz zegar
    SCU->PCGR1 |= __WIU;
    //Ustawienie rejestru maski tylko P3.4 ... P3.7 zglaszaja przerania
    WIU->MR |= 0x80;
    //Typ przerwania (zbocze opadajace) P3.4 ... P3.7
    WIU->TR &= ~0x80;
    //Odblokuj przerwanie od WIU
    WIU->CTRL = WIU_INT_EN;
    //Ustaw VIC1.10 jako linie P3.7
    SCU->WKUPSEL = WKUPSEL_EXTINT7;

    /*** Ustawienie kontrolera przerwan VIC ***/
    //Zalaczenie clocka dla VIC
    SCU->PCGR0 |= __VIC;
    //Reset
    SCU->PRR0 &= ~__VIC;
    SCU->PRR0 |= __VIC;
    //Do wektora podstaw adres procedury obslugi
    VIC1->VAiR[0] = (unsigned long)irqIntHandler;
    //Ustaw slot na przerwanie od WIUOC
    VIC1->VCiR[0] = VIC1_WIU0_BIT | VIC_SLOT_EN;
    //Zakwalifikuj przerwanie WIU jako IRQ
    VIC1->INTSR &= ~VIC1_WIU0;
    //Vic interrupt enable register
    VIC1->INTER = VIC1_WIU0;
}


/*----------------------------------------------------------------------*/
//Funkcja glowna main
int main(void)
{
    //Poczatkowe opoznienie (JTAG secure)
    delay(1000000);

    //Ustawienie portow gpio
    setup_pll();

    //Ustawienie petli PLL (czestotliwosc 96MHz)
    setup_gpio();

    //Ustawienie przerwan zewnetrznych WIU /WIC
    setup_extint();

    //Enable IRQ interrupt in ARM core
    enable_irq();
    
    //Inicjalizacja i uruchomienie wyswietlacza
    lcdInit();
    
    //Napisz na pierwszej linii wyswietlacza
    lcdPutStr("lucck@ep.com.pl");
    lcdSetPos(0x40);
    lcdPutStr("INT=");
    //Petla odczytujaca stan licznika
    while(1)
    {
	lcdSetPos(0x45);
	lcdPutInt(irqCnt);
	delay(1000000);
    }
    //Koniec main teraz przerwanie zmieni stan diody
    return 0;
}

