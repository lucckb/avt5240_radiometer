#ifndef RTC_H_
#define RTC_H_


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
    int     tm_isdst;       /* sezonowa zmiana czasu */
};


/*----------------------------------------------------------*/

//Setup RTC clock
int rtc_setup(void);

/*----------------------------------------------------------*/

//Get rtc clock
time_t rtc_get(void);

/*----------------------------------------------------------*/

//Set rtc clock
void rtc_set(time_t time);

/*----------------------------------------------------------*/

//Create mktime based on unpacked type 
time_t rtc_mktime(struct rtc_tm *tmbuf);

/*----------------------------------------------------------*/

//Get localtime base on time_t
struct rtc_tm *rtc_localtime(const time_t *timer, struct rtc_tm *tmbuf);


/*----------------------------------------------------------*/

//Create time
time_t rtc_mktime(struct rtc_tm *tmbuf);

/*----------------------------------------------------------*/

//Get gmt time
struct rtc_tm *rtc_gmtime(const time_t *timer, struct rtc_tm *tmbuf);

/*----------------------------------------------------------*/

extern int _daylight;                  // Non-zero if daylight savings time is used

extern long _dstbias;                  // Offset for Daylight Saving Time

extern long _timezone;                 // Difference in seconds between GMT and local time

/*----------------------------------------------------------*/

#endif /*RTC_H_*/
