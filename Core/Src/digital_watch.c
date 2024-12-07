/*
 * digital_watch.c
 *
 *  Created on: Nov 26, 2024
 *      Author: Yunus Emre TÜKEL
 */

#include "stm32f1xx_hal.h"
#include "digital_watch.h"

/*
 *  16x2 LCD screen is used in this project
 *	liquidcrystal_i2c library files are taken from the following link
 *	https://github.com/eziya/STM32_HAL_I2C_HD44780/blob/master/Src/
 *	Simply add liquidcrystal_i2c.h and liquidcrystal_i2c.c files to your
 * 		projects Core/Src/ and Core/Inc/ folders
 *  In liquidcrystal_i2c.h file:
 *		Change line 4 to 	#include “stm32f1xx_hal.h”
 *		Change line 58 to 	#define DEVICE_ADDR (0x27 << 1)
 *	For more details on this library:
 *	https://microcontrollerslab.com/i2c-lcd-stm32-blue-pill-stm32cubeide/
 */
#include "liquidcrystal_i2c.h"

const char *week_days[7] = {
	"SU",
	"MO",
	"TU",
	"WE",
	"TH",
	"FR",
	"SA"};

/*
 *	Variables to hold current mode and setting.
 *	This project has 4 modes and different amount of settings in different modes.
 *	For example in alarm mode sett value can be 0, 1 or 2.
 */
static uint8_t mode = 0;
static uint8_t sett = 0;

//	Flags for button press
static uint8_t button_c_pressed = 0;
static uint8_t button_l_pressed = 0;
static uint8_t button_a_pressed = 0;

//	Variables used in clock setting mode
static RTC_TimeTypeDef time_setting;
static RTC_DateTypeDef date_setting;

//	Variables used in alarm mode
static RTC_AlarmTypeDef alarm;
static volatile uint8_t alarm_triggered = 0;
static uint32_t alarm_start_time = 0;

/*
 *	Main function for digital watch
 *	Must be called in while loop in main.c
 *	Checks for button presses, resets buzzer after 20 seconds and calls functions according to current mode value
 */
void Digital_Watch_Main(RTC_HandleTypeDef *hrtc)
{
	//	To reset buzzer after alarm is triggered
	//	User should wait for 20 seconds or press button L to silence buzzer
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

	//	Main job of button C is changing mode.
	//	This part changes mode and makes adjustments before it.
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

	//	Button L changes setting in modes.
	if (button_l_pressed)
	{
		sett++;
		if (sett == 8)
			sett = 0;
		button_l_pressed = 0;
		button_a_pressed = 0;
	}

	//	Function calls according to mode
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

/*
 *	Function for timekeeping mode, default mode
 *	Prints current time, date and weekday on screen
 */
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

/*
 *	Function for alarm mode
 *	Prints current alarm time on screen
 *	Allows to change currrent alarm by pressing button L
 *	After pressing button L once hour on screen blinks and
 *		by pressing button A hour value increases
 *	Second press on button L allows to set minutes
 *	Third press sets a new alarm and returns to default alarm mode screen
 */
void Digital_Watch_Alarm_Mode(RTC_HandleTypeDef *hrtc)
{
	HD44780_SetCursor(0, 0);
	HD44780_PrintStr("Alarm");

	HD44780_SetCursor(5, 1);
	Digital_Watch_Print_Value(alarm.AlarmTime.Hours);
	HD44780_PrintStr(":");
	Digital_Watch_Print_Value(alarm.AlarmTime.Minutes);

	switch (sett)
	{
	case 1: //	Set Hours
		HAL_Delay(BLINK_DELAY_TIME);
		HD44780_SetCursor(5, 1);
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
	case 2: //	Set Minutes
		HAL_Delay(BLINK_DELAY_TIME);
		HD44780_SetCursor(8, 1);
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
	case 3: //	Set Alarm and return to default
		HAL_RTC_SetAlarm_IT(hrtc, &alarm, RTC_FORMAT_BCD);
		sett = 0;
		break;
	}
}

/*
 *	Will be added
 */
void Digital_Watch_Stopwatch_Mode()
{
	HD44780_SetCursor(0, 0);
	HD44780_PrintStr("Stopwatch");
}

/*
 *	Function for clock setting mode
 *	Prints the time on screen
 *	Value that will be set blinks
 *	Button A increases the value by 1 (not applied to seconds)
 *	Button L changes to a different value
 *	If button C is pressed while setting weekday
 *		new time and date is set and mode changed to timekeeping
 *		(code for this is in Digital_Watch_Main() function)
 *	If button C is pressed while setting a different value
 *		clock setting mode cancels
 */
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
	case 0: //	Set Seconds
		HD44780_SetCursor(6, 0);
		HD44780_PrintStr("  ");
		HAL_Delay(BLINK_DELAY_TIME);
		if (button_a_pressed)
		{
			time_setting.Seconds = 0x00;
			button_a_pressed = 0;
		}
		break;
	case 1: //	Set Hours
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
	case 2: //	Set Minutes
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
	case 3: //	Set Date
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
	case 4: //	Set Month
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
	case 5: //	Set the Tens Place of the Year
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
	case 6: //	Set the Ones Place of the Year
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
	case 7: //	Set Weekday
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

/*
 *	Welcome function that is called after power is on for the first time
 *	Must be called just before the while loop in main.c
 */
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
/*
 *	Function that rings the alarm when interrupt happens
 *	Must be called in HAL_RTC_AlarmAEventCallback() function in main.c
 */
void Digital_Watch_AlarmEventCallback()
{
	HAL_GPIO_WritePin(ALARM_GPIO_PER, ALARM_GPIO_PIN, GPIO_PIN_SET);
	alarm_triggered = 1;
	alarm_start_time = HAL_GetTick();
}

/*
 *	Function that reads button presses
 *	Must be called in HAL_GPIO_EXTI_Callback() function in main.c
 */
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

// Helper function to print values on screen
void Digital_Watch_Print_Value(uint8_t value)
{
	char str[3];
	if (value < 10)
		HD44780_PrintStr("0");
	itoa(value, str, 16);
	HD44780_PrintStr(str);
}
