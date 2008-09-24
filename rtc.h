#ifndef RTC_H_
#define RTC_H_

#include <stdint.h>

/*----------------------------------------------------------*/

typedef int time_t;

struct rtc_tm
{
    int     tm_sec;         /* sekundy */
    int     tm_min;         /* minuty */
    int     tm_hour;        /* godziny */
    int     tm_mday;        /* dzień miesiąca */
    int     tm_mon;         /* miesiąc */
    int     tm_year;        /* rok */
    int     tm_wday;        /* dzień tygodnia */
    int     tm_yday;        /* dzień roku */
};

/*----------------------------------------------------------*/

//Setup RTC clock
int rtc_setup(void);

/*----------------------------------------------------------*/

//Get rtc clock
time_t rtc_get(void);

/*----------------------------------------------------------*/

//Initialize backup domain only
void bkp_init(void);

/*----------------------------------------------------------*/

//Set rtc clock
void rtc_set(time_t time);

/*----------------------------------------------------------*/

//Write BKP registers
uint16_t rtc_bkp_read(uint8_t addr);

/*----------------------------------------------------------*/

//Write BKP registers
void rtc_bkp_write(uint8_t addr,uint16_t value);

/*----------------------------------------------------------*/

//Create mktime based on unpacked type
time_t rtc_mktime(struct rtc_tm *tmbuf);

/*----------------------------------------------------------*/

//Get localtime base on time_t
struct rtc_tm *rtc_localtime(time_t time, struct rtc_tm *tmbuf);


/*----------------------------------------------------------*/

//Create time
time_t rtc_mktime(struct rtc_tm *tmbuf);

/*----------------------------------------------------------*/

//Get gmt time
struct rtc_tm *rtc_time(time_t time, struct rtc_tm *tmbuf);

/*----------------------------------------------------------*/

extern int _daylight;                  // Non-zero if daylight savings time is used

extern long _dstbias;                  // Offset for Daylight Saving Time

extern long _timezone;                 // Difference in seconds between GMT and local time

/*----------------------------------------------------------*/

#endif /*RTC_H_*/
