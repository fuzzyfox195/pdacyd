#pragma once

#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <lcd_pins.h>

class LGFX : public lgfx::LGFX_Device
{
  lgfx::Panel_ILI9341 _panel_instance;
  lgfx::Bus_SPI _bus_instance;
  lgfx::Light_PWM _light_instance;
  lgfx::Touch_FT5x06 _touch_instance;

public:
  LGFX(void)
  {
    {
      auto cfg = _bus_instance.config();
      cfg.spi_host = SPI2_HOST;
      cfg.spi_mode = LCD_SPI_MODE;
      cfg.freq_write = LCD_FREQ_MHZ * 1000000;
      cfg.freq_read = LCD_READ_FREQ_MHZ * 1000000;
      cfg.spi_3wire = LCD_THREE_WIRE;
      cfg.use_lock = true;
      cfg.dma_channel = SPI_DMA_CH_AUTO;
      cfg.pin_sclk = LCD_SCK;
      cfg.pin_mosi = LCD_MOSI;
      cfg.pin_miso = LCD_MISO;
      cfg.pin_dc = LCD_DC;

      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }

    {
      auto cfg = _panel_instance.config();
      cfg.pin_cs = LCD_CS;
      cfg.pin_rst = LCD_RST;
      cfg.pin_busy = -1;
      cfg.panel_width = LCD_H_RES;
      cfg.panel_height = LCD_V_RES;
      cfg.memory_width = LCD_H_RES;
      cfg.memory_height = LCD_V_RES;
      cfg.offset_x = 0;
      cfg.offset_y = 0;
      cfg.offset_rotation = 0;
      cfg.dummy_read_pixel = 8;
      cfg.dummy_read_bits = 1;
      cfg.readable = true;
      cfg.invert = LCD_INVERT;
      cfg.rgb_order = LCD_RGB_ORDER;
      cfg.dlen_16bit = false;
      cfg.bus_shared = LCD_BUS_SHARED;

      _panel_instance.config(cfg);
    }

    {
      auto cfg = _light_instance.config();
      cfg.pin_bl = LCD_BL_PIN;
      cfg.invert = LCD_BL_ON_LEVEL == 0;
      cfg.freq = LCD_BL_FREQ;
      cfg.pwm_channel = LCD_BL_PWM_CHANNEL;

      _light_instance.config(cfg);
      _panel_instance.setLight(&_light_instance);
    }

    {
      auto cfg = _touch_instance.config();
      cfg.x_min = 0;
      cfg.x_max = LCD_H_RES - 1;
      cfg.y_min = 0;
      cfg.y_max = LCD_V_RES - 1;
      cfg.pin_int = TOUCH_INT;
      cfg.pin_rst = TOUCH_RST;
      cfg.pin_sda = TOUCH_SDA;
      cfg.pin_scl = TOUCH_SCL;
      cfg.bus_shared = false;
      cfg.offset_rotation = 0;
      cfg.i2c_port = TOUCH_I2C_PORT;
      cfg.i2c_addr = TOUCH_I2C_ADDR;
      cfg.freq = TOUCH_I2C_FREQ;

      _touch_instance.config(cfg);
      _panel_instance.setTouch(&_touch_instance);
    }

    setPanel(&_panel_instance);
  }
};