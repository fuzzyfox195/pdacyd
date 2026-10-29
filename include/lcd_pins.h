// LCDWIKI 2.8" ESP32-S3 (ES3C28P): ILI9341 + FT6236/FT6336U
#pragma once

#define LCD_H_RES        240
#define LCD_V_RES        320
#define LCD_ROTATION     1
#define SCREEN_WIDTH     320
#define SCREEN_HEIGHT    240
#define LCD_COLOR_DEPTH  16

#define LCD_SCK          12
#define LCD_MOSI         11
#define LCD_MISO         13
#define LCD_CS           10
#define LCD_DC           46
#define LCD_RST          -1
#define LCD_FREQ_MHZ     60
#define LCD_READ_FREQ_MHZ 16
#define LCD_SPI_MODE     0
#define LCD_THREE_WIRE   0
#define LCD_INVERT       1
#define LCD_RGB_ORDER    0
#define LCD_BUS_SHARED   1

#define LCD_BL_PIN       45
#define LCD_BL_ON_LEVEL  1
#define LCD_BL_FREQ      5000
#define LCD_BL_PWM_CHANNEL 7
#define LCD_BL_DEFAULT   100

#define TOUCH_SDA        16
#define TOUCH_SCL        15
#define TOUCH_INT        17
#define TOUCH_RST        18
#define TOUCH_I2C_PORT   0
#define TOUCH_I2C_ADDR   0x38
#define TOUCH_I2C_FREQ   400000
#define TOUCH_SWAP_XY    0
#define TOUCH_INVERT_X   1
#define TOUCH_INVERT_Y   1
