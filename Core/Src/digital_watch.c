/*
 * digital_watch.c
 *
 *  Created on: Nov 26, 2024
 *      Author: Yunus Emre TÜKEL
 */

#include "digital_watch.h"
#include "liquidcrystal_i2c.h"
#include "stm32f1xx_hal.h"

const char *week_days[7] = {
	"SU",
	"MO",
	"TU",
	"WE",
	"TH",
	"FR",
	"SA"};

static uint8_t mode = 0;
static uint8_t sett = 0;

static uint8_t button_c_pressed = 0;
static uint8_t button_l_pressed = 0;
static uint8_t button_a_pressed = 0;

static RTC_TimeTypeDef time_setting;
static RTC_DateTypeDef date_setting;

static RTC_AlarmTypeDef alarm;
static volatile uint8_t alarm_triggered = 0;
static uint32_t alarm_start_time = 0;

void Digital_Watch_Print_Value(uint8_t value)
{
	char str[3];
	if (value < 10)
		HD44780_PrintStr("0");
	itoa(value, str, 16);
	HD44780_PrintStr(str);
}

void Digital_Watch_Main(RTC_HandleTypeDef *hrtc)
{
	if (alarm_triggered)
	{
		if ((HAL_GetTick() - alarm_start_time) >= 20000)
		{
			HAL_GPIO_WritePin(ALARM_GPIO_PER, ALARM_GPIO_PIN, GPIO_PIN_RESET);
			alarm_triggered = 0;
		}
		if (button_l_pressed)
		{
			HAL_GPIO_WritePin(ALARM_GPIO_PER, ALARM_GPIO_PIN, GPIO_PIN_RESET);
			alarm_triggered = 0;
			button_l_pressed = 0;
		}
	}
	if (button_c_pressed)
	{
		HD44780_Clear();
		mode++;
		if (mode == 1)
		{
			HAL_RTC_GetAlarm(hrtc, &alarm, RTC_ALARM_A, RTC_FORMAT_BCD);
		}
		if (mode == 3)
		{
			HAL_RTC_GetTime(hrtc, &time_setting, RTC_FORMAT_BCD);
			HAL_RTC_GetDate(hrtc, &date_setting, RTC_FORMAT_BCD);
		}
		if (mode == 4)
		{
			if (sett == 7)
			{
				HAL_RTC_SetTime(hrtc, &time_setting, RTC_FORMAT_BCD);
				HAL_RTC_SetDate(hrtc, &date_setting, RTC_FORMAT_BCD);
			}
			mode = 0;
		}
		button_c_pressed = 0;
		button_l_pressed = 0;
		button_a_pressed = 0;
		sett = 0;
	}
	if (button_l_pressed)
	{
		sett++;
		if (sett == 8)
			sett = 0;
		button_l_pressed = 0;
		button_a_pressed = 0;
	}
	switch (mode)
	{
	case 0:
		Digital_Watch_Timekeeping_Mode(hrtc);
		break;
	case 1:
		Digital_Watch_Alarm_Mode(hrtc);
		break;
	case 2:
		Digital_Watch_Stopwatch_Mode();
		break;
	case 3:
		Digital_Watch_Setting_Mode();
		break;
	}
}

void Digital_Watch_Timekeeping_Mode(RTC_HandleTypeDef *hrtc)
{
	RTC_TimeTypeDef time;
	RTC_DateTypeDef date;

	HAL_RTC_GetTime(hrtc, &time, RTC_FORMAT_BCD);
	HAL_RTC_GetDate(hrtc, &date, RTC_FORMAT_BCD);

	HD44780_SetCursor(0, 0);
	Digital_Watch_Print_Value(time.Hours);
	HD44780_PrintStr(":");
	Digital_Watch_Print_Value(time.Minutes);
	HD44780_PrintStr(":");
	Digital_Watch_Print_Value(time.Seconds);

	HD44780_SetCursor(0, 1);
	Digital_Watch_Print_Value(date.Date);
	HD44780_PrintStr("/");
	Digital_Watch_Print_Value(date.Month);
	HD44780_PrintStr("/");
	HD44780_PrintStr("20");
	Digital_Watch_Print_Value(date.Year);

	HD44780_PrintStr("    ");
	HD44780_PrintStr(week_days[date.WeekDay]);
}

void Digital_Watch_Alarm_Mode(RTC_HandleTypeDef *hrtc)
{
	HD44780_SetCursor(0, 0);
	HD44780_PrintStr("Alarm");

	HD44780_SetCursor(4, 1);
	Digital_Watch_Print_Value(alarm.AlarmTime.Hours);
	HD44780_PrintStr(":");
	Digital_Watch_Print_Value(alarm.AlarmTime.Minutes);

	switch (sett)
	{
	case 1:
		HAL_Delay(BLINK_DELAY_TIME);
		HD44780_SetCursor(0, 1);
		HD44780_PrintStr("  ");
		HAL_Delay(BLINK_DELAY_TIME);
		if (button_a_pressed)
		{
			alarm.AlarmTime.Hours++;
			if (alarm.AlarmTime.Hours == 0x24)
				alarm.AlarmTime.Hours = 0x00;
			if ((alarm.AlarmTime.Hours % 0x10) == 0x0a)
				alarm.AlarmTime.Hours += 0x06;
			button_a_pressed = 0;
		}
		break;
	case 2:
		HAL_Delay(BLINK_DELAY_TIME);
		HD44780_SetCursor(3, 1);
		HD44780_PrintStr("  ");
		HAL_Delay(BLINK_DELAY_TIME);
		if (button_a_pressed)
		{
			alarm.AlarmTime.Minutes++;
			if (alarm.AlarmTime.Minutes == 0x5a)
				alarm.AlarmTime.Minutes = 0x00;
			if ((alarm.AlarmTime.Minutes % 0x10) == 0x0a)
				alarm.AlarmTime.Minutes += 0x06;
			button_a_pressed = 0;
		}
		break;
	case 3:
		HAL_RTC_SetAlarm_IT(hrtc, &alarm, RTC_FORMAT_BCD);
		sett = 0;
		break;
	}
}

void Digital_Watch_Stopwatch_Mode()
{
	HD44780_SetCursor(0, 0);
	HD44780_PrintStr("Stopwatch");
}

void Digital_Watch_Setting_Mode()
{
	HD44780_SetCursor(0, 0);
	Digital_Watch_Print_Value(time_setting.Hours);
	HD44780_PrintStr(":");
	Digital_Watch_Print_Value(time_setting.Minutes);
	HD44780_PrintStr(":");
	Digital_Watch_Print_Value(time_setting.Seconds);

	HD44780_SetCursor(0, 1);
	Digital_Watch_Print_Value(date_setting.Date);
	HD44780_PrintStr("/");
	Digital_Watch_Print_Value(date_setting.Month);
	HD44780_PrintStr("/");
	HD44780_PrintStr("20");
	Digital_Watch_Print_Value(date_setting.Year);

	HD44780_PrintStr("    ");
	HD44780_PrintStr(week_days[date_setting.WeekDay]);

	HAL_Delay(BLINK_DELAY_TIME);
	switch (sett)
	{
	case 0:
		HD44780_SetCursor(6, 0);
		HD44780_PrintStr("  ");
		HAL_Delay(BLINK_DELAY_TIME);
		if (button_a_pressed)
		{
			time_setting.Seconds = 0x00;
			button_a_pressed = 0;
		}
		break;
	case 1:
		HD44780_SetCursor(0, 0);
		HD44780_PrintStr("  ");
		HAL_Delay(BLINK_DELAY_TIME);
		if (button_a_pressed)
		{
			time_setting.Hours++;
			if (time_setting.Hours == 0x24)
				time_setting.Hours = 0x00;
			if ((time_setting.Hours % 0x10) == 0x0a)
				time_setting.Hours += 0x06;
			button_a_pressed = 0;
		}
		break;
	case 2:
		HD44780_SetCursor(3, 0);
		HD44780_PrintStr("  ");
		HAL_Delay(BLINK_DELAY_TIME);
		if (button_a_pressed)
		{
			time_setting.Minutes++;
			if (time_setting.Minutes == 0x5a)
				time_setting.Minutes = 0x00;
			if ((time_setting.Minutes % 0x10) == 0x0a)
				time_setting.Minutes += 0x06;
			button_a_pressed = 0;
		}
		break;
	case 3:
		HD44780_SetCursor(0, 1);
		HD44780_PrintStr("  ");
		HAL_Delay(BLINK_DELAY_TIME);
		if (button_a_pressed)
		{
			date_setting.Date++;
			if (date_setting.Date == 0x32)
				date_setting.Date = 0x01;
			if ((date_setting.Date % 0x10) == 0x0a)
				date_setting.Date += 0x06;
			button_a_pressed = 0;
		}
		break;
	case 4:
		HD44780_SetCursor(3, 1);
		HD44780_PrintStr("  ");
		HAL_Delay(BLINK_DELAY_TIME);
		if (button_a_pressed)
		{
			date_setting.Month++;
			if (date_setting.Month == 0x13)
				date_setting.Month = 0x01;
			if (date_setting.Month == 0x0a)
				date_setting.Month = 0x10;
			button_a_pressed = 0;
		}
		break;
	case 5:
		HD44780_SetCursor(8, 1);
		HD44780_PrintStr(" ");
		HAL_Delay(BLINK_DELAY_TIME);
		if (button_a_pressed)
		{
			date_setting.Year += 0x10;
			if (date_setting.Year > 0x99)
				date_setting.Year -= 0xa0;
			button_a_pressed = 0;
		}
		break;
	case 6:
		HD44780_SetCursor(9, 1);
		HD44780_PrintStr(" ");
		HAL_Delay(BLINK_DELAY_TIME);
		if (button_a_pressed)
		{
			date_setting.Year++;
			if ((date_setting.Year % 0x10) == 0x0a)
				date_setting.Year -= 0x0a;
			button_a_pressed = 0;
		}
		break;
	case 7:
		HD44780_SetCursor(14, 1);
		HD44780_PrintStr("  ");
		HAL_Delay(BLINK_DELAY_TIME);
		if (button_a_pressed)
		{
			date_setting.WeekDay++;
			if (date_setting.WeekDay == 0x07)
				date_setting.WeekDay = 0x00;
			button_a_pressed = 0;
		}
		break;
	}
}

void Digital_Watch_Welcome()
{
	HD44780_Init(2);
	HD44780_Clear();
	HD44780_SetCursor(0, 0);
	HD44780_PrintStr(" Digital  Watch ");
	HD44780_SetCursor(0, 1);
	HD44780_PrintStr("Yunus Emre TUKEL");
	HAL_Delay(3000);
	HD44780_Clear();
}

void Digital_Watch_AlarmEventCallback()
{
	HAL_GPIO_WritePin(ALARM_GPIO_PER, ALARM_GPIO_PIN, GPIO_PIN_SET);
	alarm_triggered = 1;
	alarm_start_time = HAL_GetTick();
}

void Digital_Watch_Button_Pressed(uint16_t GPIO_Pin)
{
	switch (GPIO_Pin)
	{
	case BUTTON_C_GPIO_PIN:
		button_c_pressed = 1;
		break;
	case BUTTON_L_GPIO_PIN:
		button_l_pressed = 1;
		break;
	case BUTTON_A_GPIO_PIN:
		button_a_pressed = 1;
		break;
	}
}
