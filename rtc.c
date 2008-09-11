#include "rtc.h"
#include "stm32f10x_lib.h"
#include <stdint.h>
#include "system.h"

/*----------------------------------------------------------*/

//PWR control register
#define PWR_CR (*(volatile uint32_t*)0x40007000) 
//Enable access to backup domain and RTC
#define PWR_CR_DBP (1<<8)
//Enable LSE oscilator
#define BDCR_LSEON 1
//LSE ready flag
#define BDCR_LSERDY (1<<1)
//LSE clock source mask
#define BDCR_RTC_SRC_MASK (~((1<<8)|1<<9))
//Enable RTC clock bit
#define BDCR_RTC_EN (1<<15)
//Configuration Flag Enable Mask
#define CRL_CNF_Set      ((u16)0x0010)       
//Configuration Flag Disable Mask
#define CRL_CNF_Reset    ((u16)0xFFEF)       

/*----------------------------------------------------------*/
//Wait for sync
static void rtc_wait_for_sync(void)
{
	 // Clear RSF flag 
	  RTC->CRL &= ~RTC_FLAG_RSF;
	 // Loop until RSF flag is set 
	  while ((RTC->CRL & RTC_FLAG_RSF) == 0);
}

/*----------------------------------------------------------*/
//Wait for last write
static void rtc_wait_for_last_write(void)
{
	while( !(RTC->CRL & RTC_FLAG_RTOFF) );
}

/*----------------------------------------------------------*/
//Setup RTC clock
int rtc_setup(void)
{
	//Enable clock for power and backup domains
	RCC->APB1ENR |= RCC_APB1Periph_PWR | RCC_APB1Periph_BKP;
	//Enable acces to RTC and backup regs
	PWR_CR |= PWR_CR_DBP;
	//Disable LSE oscilator
	RCC->BDCR &= ~BDCR_LSEON;
	asm volatile("nop");
	//Enable LSE oscilator
	RCC->BDCR |= BDCR_LSEON;
	//Wait for lserdy flag
	int timeout=50000;
	while( !(RCC->BDCR & BDCR_LSERDY) )
		if(--timeout==0) return EXIT_FAILURE;
	//Enable LSE as RTC clock source
	RCC->BDCR &= BDCR_RTC_SRC_MASK;
	RCC->BDCR |= RCC_RTCCLKSource_LSE;
	//Enable RTC clock
	RCC->BDCR |= BDCR_RTC_EN;
	
	//Wait for synchronization
	rtc_wait_for_sync();
	//Wait for write
	rtc_wait_for_last_write();
	
	/* Enable configuration */
	 RTC->CRL |= CRL_CNF_Set;
	//RTC set prescaler 
	RTC->PRLH = 0;
	RTC->PRLL = 32768;
	/* Disable configuration */
    RTC->CRL &= CRL_CNF_Reset;
    
    //Wait for write
    rtc_wait_for_last_write();
    
    
	return EXIT_SUCCESS;
}

/*----------------------------------------------------------*/
//Get rtc clock
time_t rtc_get(void)
{
	return RTC->CNTL | ((uint32_t)RTC->CNTH)<<16;
}

/*----------------------------------------------------------*/

//Set rtc clock
void rtc_set(time_t time)
{
	rtc_wait_for_last_write();
	/* Enable configuration */
	RTC->CRL |= CRL_CNF_Set;
	//Set counter
	RTC->CNTH = time >> 16;
	RTC->CNTL = time;
	/* Disable configuration */
	RTC->CRL &= CRL_CNF_Reset;
}

/*----------------------------------------------------------*/

#define YEAR0                   1900
#define EPOCH_YR                1970
#define SECS_DAY                (24L * 60L * 60L)
#define LEAPYEAR(year)          (!((year) % 4) && (((year) % 100) || !((year) % 400)))
#define YEARSIZE(year)          (LEAPYEAR(year) ? 366 : 365)
#define FIRSTSUNDAY(timp)       (((timp)->tm_yday - (timp)->tm_wday + 420) % 7)
#define FIRSTDAYOF(timp)        (((timp)->tm_wday - (timp)->tm_yday + 420) % 7)

#define TIME_MAX                2147483647L

int _daylight = 0;                  // Non-zero if daylight savings time is used
long _dstbias = 0;                  // Offset for Daylight Saving Time
long _timezone = 0;                 // Difference in seconds between GMT and local time

/*----------------------------------------------------------*/

static const int _ytab[2][12] = 
{
  {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
  {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
};

/*----------------------------------------------------------*/

struct rtc_tm *rtc_gmtime(const time_t *timer, struct rtc_tm *tmbuf)
{
  time_t time = *timer;
  unsigned long dayclock, dayno;
  int year = EPOCH_YR;

  dayclock = (unsigned long) time % SECS_DAY;
  dayno = (unsigned long) time / SECS_DAY;

  tmbuf->tm_sec = dayclock % 60;
  tmbuf->tm_min = (dayclock % 3600) / 60;
  tmbuf->tm_hour = dayclock / 3600;
  tmbuf->tm_wday = (dayno + 4) % 7; // Day 0 was a thursday
  while (dayno >= (unsigned long) YEARSIZE(year)) 
  {
    dayno -= YEARSIZE(year);
    year++;
  }
  tmbuf->tm_year = year - YEAR0;
  tmbuf->tm_yday = dayno;
  tmbuf->tm_mon = 0;
  while (dayno >= (unsigned long) _ytab[LEAPYEAR(year)][tmbuf->tm_mon]) 
  {
    dayno -= _ytab[LEAPYEAR(year)][tmbuf->tm_mon];
    tmbuf->tm_mon++;
  }
  tmbuf->tm_mday = dayno + 1;
  tmbuf->tm_isdst = 0;

  return tmbuf;
}

/*----------------------------------------------------------*/
struct rtc_tm *rtc_localtime(const time_t *timer, struct rtc_tm *tmbuf)
{
  time_t t;

  t = *timer - _timezone;
  return rtc_gmtime(&t, tmbuf);
}

/*----------------------------------------------------------*/
time_t rtc_mktime(struct rtc_tm *tmbuf)
{
  long day, year;
  int tm_year;
  int yday, month;
  /*unsigned*/ long seconds;
  int overflow;
  long dst;

  tmbuf->tm_min += tmbuf->tm_sec / 60;
  tmbuf->tm_sec %= 60;
  if (tmbuf->tm_sec < 0) 
  {
    tmbuf->tm_sec += 60;
    tmbuf->tm_min--;
  }
  tmbuf->tm_hour += tmbuf->tm_min / 60;
  tmbuf->tm_min = tmbuf->tm_min % 60;
  if (tmbuf->tm_min < 0) 
  {
    tmbuf->tm_min += 60;
    tmbuf->tm_hour--;
  }
  day = tmbuf->tm_hour / 24;
  tmbuf->tm_hour= tmbuf->tm_hour % 24;
  if (tmbuf->tm_hour < 0) 
  {
    tmbuf->tm_hour += 24;
    day--;
  }
  tmbuf->tm_year += tmbuf->tm_mon / 12;
  tmbuf->tm_mon %= 12;
  if (tmbuf->tm_mon < 0) 
  {
    tmbuf->tm_mon += 12;
    tmbuf->tm_year--;
  }
  day += (tmbuf->tm_mday - 1);
  while (day < 0) 
  {
    if(--tmbuf->tm_mon < 0) 
    {
      tmbuf->tm_year--;
      tmbuf->tm_mon = 11;
    }
    day += _ytab[LEAPYEAR(YEAR0 + tmbuf->tm_year)][tmbuf->tm_mon];
  }
  while (day >= _ytab[LEAPYEAR(YEAR0 + tmbuf->tm_year)][tmbuf->tm_mon]) 
  {
    day -= _ytab[LEAPYEAR(YEAR0 + tmbuf->tm_year)][tmbuf->tm_mon];
    if (++(tmbuf->tm_mon) == 12) 
    {
      tmbuf->tm_mon = 0;
      tmbuf->tm_year++;
    }
  }
  tmbuf->tm_mday = day + 1;
  year = EPOCH_YR;
  if (tmbuf->tm_year < year - YEAR0) return (time_t) -1;
  seconds = 0;
  day = 0;                      // Means days since day 0 now
  overflow = 0;

  // Assume that when day becomes negative, there will certainly
  // be overflow on seconds.
  // The check for overflow needs not to be done for leapyears
  // divisible by 400.
  // The code only works when year (1970) is not a leapyear.
  tm_year = tmbuf->tm_year + YEAR0;

  if (TIME_MAX / 365 < tm_year - year) overflow++;
  day = (tm_year - year) * 365;
  if (TIME_MAX - day < (tm_year - year) / 4 + 1) overflow++;
  day += (tm_year - year) / 4 + ((tm_year % 4) && tm_year % 4 < year % 4);
  day -= (tm_year - year) / 100 + ((tm_year % 100) && tm_year % 100 < year % 100);
  day += (tm_year - year) / 400 + ((tm_year % 400) && tm_year % 400 < year % 400);

  yday = month = 0;
  while (month < tmbuf->tm_mon)
  {
    yday += _ytab[LEAPYEAR(tm_year)][month];
    month++;
  }
  yday += (tmbuf->tm_mday - 1);
  if (day + yday < 0) overflow++;
  day += yday;

  tmbuf->tm_yday = yday;
  tmbuf->tm_wday = (day + 4) % 7;               // Day 0 was thursday (4)

  seconds = ((tmbuf->tm_hour * 60L) + tmbuf->tm_min) * 60L + tmbuf->tm_sec;

  if ((TIME_MAX - seconds) / SECS_DAY < day) overflow++;
  seconds += day * SECS_DAY;

  // Now adjust according to timezone and daylight saving time
  if (((_timezone > 0) && (TIME_MAX - _timezone < seconds))
      || ((_timezone < 0) && (seconds < -_timezone)))
          overflow++;
  seconds += _timezone;

  if (tmbuf->tm_isdst)
    dst = _dstbias;
  else 
    dst = 0;

  if (dst > seconds) overflow++;        // dst is always non-negative
  seconds -= dst;

  if (overflow) return (time_t) -1;

  if ((time_t) seconds != seconds) return (time_t) -1;
  return (time_t) seconds;
}

/*----------------------------------------------------------*/

