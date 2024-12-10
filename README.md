# Digital Watch Project for STM32F103C6T6A

This project is a **digital watch** implementation inspired by the Casio watch described in [this manual](https://www.casio.com/content/dam/casio/global/support/manuals/watches/pdf/5/593/qw593_EN.pdf). The controls are nearly identical to the Casio model, and the watch includes advanced features like time and date display, alarm setting, and manual time adjustment.

## Features
- Displays the current time in `HH:MM:SS` format.
- Displays the date in `DD.MM.YYYY` format and the day of the week.
- Allows users to set alarms.
- Manual time and date adjustment.
- (Upcoming) Stopwatch functionality.

## Requirements
### Hardware
- STM32F103C6T6A board
- 16x2 LCD with I2C converter
- Buzzer (for alarm notifications)
- 3 push buttons
- Breadboard and jumper cables
- FTDI board (for programming)

### Software
- STM32CubeIDE (for development and debugging)
- STM32CubeProgrammer (for flashing the firmware)

## Circuit Connections
| STM32 Pin  | Component   | Description                |
|------------|-------------|----------------------------|
| A1         | Button C    | Control button             |
| A2         | Button L    | Mode selection button      |
| A3         | Button A    | Adjust button              |
| A15        | Buzzer      | Alarm sound output         |
| B6 and B7  | 16x2 LCD    | Displays time and date     |

## STM32CubeIDE Configuration

Below are the initial configurations made in STM32CubeIDE to set up the project for the STM32F103C6T6A board:

### 1. RCC (Reset and Clock Control)
- **System Core > RCC**: Set the clock source to **HSE crystal/ceramic resonator**.

### 2. RTC (Real-Time Clock)
- **Timers > RTC**:
  - Enable **Activate clock source** and **Activate calendar** options.
  - Set **RTC Output** to **RTC output on the tamper pin**.

### 3. I2C
- **Connectivity > I2C1**: Enable I2C1 for communication with the LCD.

### 4. GPIO (General Purpose Input/Output)
- **System Core > GPIO**:
  - **PA1, PA2, PA3**: Configured as:
    - Mode: **External Interrupt mode with rising edge trigger detection**
    - Pull: **Pull-down**
  - **PA15**: Configured as:
    - Mode: **Output push-pull**
    - Pull: **Pull-down**

### 5. NVIC (Nested Vector Interrupt Controller)
- **System Core > NVIC**: Enable the following interrupts:
  - **RTC global interrupt**
  - **EXTI line1, line2, and line3 interrupt**
  - **RTC alarm interrupt through EXTI line 17**

## Using the LiquidCrystal_I2C Library

This project uses a **16x2 LCD screen** with an I2C interface to display the current time and date. The LCD is managed using the `LiquidCrystal_I2C` library. Follow the steps below to include and configure the library:

### 1. Download the Library
The library files can be downloaded from the following link:
[LiquidCrystal_I2C GitHub Repository](https://github.com/eziya/STM32_HAL_I2C_HD44780/blob/master/Src/)

### 2. Add the Files to Your Project
Copy the `liquidcrystal_i2c.h` and `liquidcrystal_i2c.c` files into the appropriate folders in your STM32CubeIDE project:
- **Core/Inc/**: Place `liquidcrystal_i2c.h`
- **Core/Src/**: Place `liquidcrystal_i2c.c`

### 3. Modify the Header File
Make the following changes to the `liquidcrystal_i2c.h` file:
- **Line 4**: Replace with:
  ```c
  #include "stm32f1xx_hal.h"
- **Line 58**: Replace with:
  ```c
  #define DEVICE_ADDR (0x27 << 1)
