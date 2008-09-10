#ifndef RTC_H_
#define RTC_H_
#include <time.h>


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

#endif /*RTC_H_*/
