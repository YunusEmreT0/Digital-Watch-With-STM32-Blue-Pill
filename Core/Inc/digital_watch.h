/*
 * digital_watch.h
 *
 *  Created on: Nov 26, 2024
 *      Author: Yunus Emre TÜKEL
 */

#ifndef INC_DIGITAL_WATCH_H_
#define INC_DIGITAL_WATCH_H_

#define ALARM_GPIO_PER GPIOA
#define ALARM_GPIO_PIN GPIO_PIN_15

#define BUTTON_C_GPIO_PIN GPIO_PIN_1
#define BUTTON_L_GPIO_PIN GPIO_PIN_2
#define BUTTON_A_GPIO_PIN GPIO_PIN_3

#define BLINK_DELAY_TIME 300

void Digital_Watch_Print_Value(uint8_t value);
void Digital_Watch_Main(RTC_HandleTypeDef *hrtc);
void Digital_Watch_Timekeeping_Mode(RTC_HandleTypeDef *hrtc);
void Digital_Watch_Alarm_Mode(RTC_HandleTypeDef *hrtc);
void Digital_Watch_Stopwatch_Mode();
void Digital_Watch_Setting_Mode();
void Digital_Watch_Welcome();
void Digital_Watch_AlarmEventCallback();
void Digital_Watch_Button_Pressed(uint16_t GPIO_Pin);

#endif /* INC_DIGITAL_WATCH_H_ */
