#include "rtc.h"
#include "ints.h"

Byte data_mode = 0;
/** converts BCD (Binary Coded Decimal) to decimal
 */
Byte bcd2dec(Byte i)
{
	return (((i >> 4) & 0x0F) * 10 + (i & 0x0F));
}

/** converts decimal to BCD (Binary Coded Decimal)
 */
Byte dec2bcd(Byte i)
{
	return (((i/10) << 4) + (i % 10));
}

/** Wait until data in rtc is valid.
 * Data is valid until +/- 240 usec after this function returns,
 * thus a read/write can be done to the RTC within this time period
 */
void rtc_valid()
{
	int ready = 0;
	
	while(!ready)
	{
		Byte stat_a = read_rtc(RTC_STAT_A);
		if(!(stat_a & RTC_UIP))
			ready = 1;
	}
}

/** Returns rtc data from I/O address add. Doesn't check valid bit
 */
Byte read_rtc(Byte add)
{
	disable_irq(RTC_IRQ);
	outportb(RTC_ADDR_REG, add);
	Byte val = inportb(RTC_DATA_REG);
	enable_irq(RTC_IRQ);
	
	return val;
}

/** Returns rtc data from I/O address add. Check valid bit
 */
Byte read_rtcv(Byte add)
{
	rtc_valid();
	return read_rtc(add);
}

/** Write value to I/O address add. Doesn't check valid bit
 */
void write_rtc(Byte add, Byte value)
{
	disable_irq(RTC_IRQ);
	outportb(RTC_ADDR_REG, add);
	outportb(RTC_DATA_REG, value);
	enable_irq(RTC_IRQ);
}

/** Write value to I/O address add. Check valid bit
 */
void write_rtcv(Byte add, Byte value)
{
	rtc_valid();
	write_rtc(add, value);
}

/** Read RTC stored time
 *
 * Uses read_rtcv() and bcd2dec()
 */
void rtc_read_time(RTC_TIME *time)
{
	if(!data_mode)
	{
	time->sec = bcd2dec(read_rtcv(SEC));
	time->min = bcd2dec(read_rtcv(MIN));
	time->hour = bcd2dec(read_rtcv(HOUR));
	}
	else
	{
		time->sec = read_rtcv(SEC);
		time->min = read_rtcv(MIN);
		time->hour =read_rtcv(HOUR);
	}
}

/** Read RTC stored date
 *
 * Uses read_rtcv() and bcd2dec()
 */
void rtc_read_date(RTC_DATE *date)
{
	if(!data_mode)
	{
		date->day = bcd2dec(read_rtcv(MONTH_DAY));
		date->month = bcd2dec(read_rtcv(MONTH));
		date->year = bcd2dec(read_rtcv(YEAR));
	}
	else
	{
		date->day = read_rtcv(MONTH_DAY);
		date->month = read_rtcv(MONTH);
		date->year = read_rtcv(YEAR);
	}
}

/** Read RTC stored alarm
 *
 * Uses read_rtcv() and bcd2dec()
 */
void rtc_read_alarm(RTC_TIME *time)
{
	if(!data_mode)
	{
		time->sec = bcd2dec(read_rtcv(SEC_ALARM));
		time->min = bcd2dec(read_rtcv(MIN_ALARM));
		time->hour = bcd2dec(read_rtcv(HOUR_ALARM));
	}
	else
	{
		time->sec = read_rtcv(SEC_ALARM);
		time->min = read_rtcv(MIN_ALARM);
		time->hour = read_rtcv(HOUR_ALARM);
	}
}

/** Write alarm to RTC
 *
 * Uses write_rtcv() and dec2bcd()
 */
void rtc_write_alarm(RTC_TIME *time)
{
	if(!data_mode)
	{
		write_rtcv(SEC_ALARM, dec2bcd(time->sec));
		write_rtcv(MIN_ALARM, dec2bcd(time->min));
		write_rtcv(HOUR_ALARM, dec2bcd(time->hour));
	}
	else
	{
		write_rtcv(SEC_ALARM, time->sec);
		write_rtcv(MIN_ALARM, time->min);
		write_rtcv(HOUR_ALARM, time->hour);
	}
}
