#include <Arduino.h>
#include <esp_heap_caps.h>
#include <lvgl.h>
#include <LGFX_config.hpp>
#include <lcd_pins.h>
#include "ui/main_ui.h"

LGFX lcd;

static constexpr uint32_t DRAW_BUF_LINES = 60;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t *draw_buf_1 = nullptr;
static lv_color_t *draw_buf_2 = nullptr;
static lv_disp_drv_t *pending_flush_driver = nullptr;
static bool flush_pending = false;

void my_disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
    const uint32_t width = area->x2 - area->x1 + 1;
    const uint32_t height = area->y2 - area->y1 + 1;
    lcd.startWrite();
    lcd.setAddrWindow(area->x1, area->y1, width, height);
    lcd.writePixelsDMA(
        reinterpret_cast<const uint16_t *>(color_p),
        width * height,
        LV_COLOR_16_SWAP == 0);
    lcd.endWrite();
    pending_flush_driver = disp_drv;
    flush_pending = true;
}

static void my_disp_wait(lv_disp_drv_t *disp_drv)
{
    while (flush_pending && lcd.dmaBusy()) {
        delay(1);
    }

    if (flush_pending) {
        pending_flush_driver = nullptr;
        flush_pending = false;
        lv_disp_flush_ready(disp_drv);
    }
}

void my_touch_read(lv_indev_drv_t *, lv_indev_data_t *data)
{
    uint16_t touch_x, touch_y;
    if (lcd.getTouch(&touch_x, &touch_y)) {
        int32_t x = touch_x;
        int32_t y = touch_y;
        #if TOUCH_SWAP_XY
        const int32_t tmp = x;
        x = y;
        y = tmp;
        #endif
        #if TOUCH_INVERT_X
        x = SCREEN_WIDTH - 1 - x;
        #endif
        #if TOUCH_INVERT_Y
        y = SCREEN_HEIGHT - 1 - y;
        #endif
        data->state = LV_INDEV_STATE_PR;
        data->point.x = x;
        data->point.y = y;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("Starting LVGL with LovyanGFX on ESP32-S3...");

    if (!psramFound()) {
        Serial.println("PSRAM is required by the LVGL memory pool but was not detected.");
        while (true) delay(1000);
    }

    lcd.setColorDepth(LCD_COLOR_DEPTH);
    if (!lcd.init()) {
        Serial.println("LCD initialization failed; check the panel and pin configuration.");
        while (true) delay(1000);
    }
    lcd.setRotation(LCD_ROTATION);
    lcd.setBrightness(255 * LCD_BL_DEFAULT / 100);

    const size_t draw_buf_pixels = SCREEN_WIDTH * DRAW_BUF_LINES;
    const size_t draw_buf_bytes = draw_buf_pixels * sizeof(lv_color_t);
    draw_buf_1 = static_cast<lv_color_t *>(
        heap_caps_malloc(draw_buf_bytes, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL));
    draw_buf_2 = static_cast<lv_color_t *>(
        heap_caps_malloc(draw_buf_bytes, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL));
    if (draw_buf_1 == nullptr || draw_buf_2 == nullptr) {
        heap_caps_free(draw_buf_1);
        heap_caps_free(draw_buf_2);
        Serial.println("Could not allocate two DMA-capable LVGL draw buffers in internal RAM.");
        while (true) delay(1000);
    }

    lv_init();
    lv_disp_draw_buf_init(&draw_buf, draw_buf_1, draw_buf_2, draw_buf_pixels);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.draw_buf = &draw_buf;
    disp_drv.hor_res = SCREEN_WIDTH;
    disp_drv.ver_res = SCREEN_HEIGHT;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.wait_cb = my_disp_wait;
    lv_disp_drv_register(&disp_drv);

    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touch_read;
    lv_indev_drv_register(&indev_drv);
    ui_init();
}

void loop() {
    static uint32_t last_tick = millis();
    const uint32_t now = millis();
    lv_tick_inc(now - last_tick);
    last_tick = now;

    if (flush_pending && !lcd.dmaBusy()) {
        lv_disp_drv_t *completed_driver = pending_flush_driver;
        pending_flush_driver = nullptr;
        flush_pending = false;
        lv_disp_flush_ready(completed_driver);
    }

    uint32_t wait_ms = lv_timer_handler();

    if (flush_pending) {
        wait_ms = 1;
    } else {
        if (wait_ms == 0) {
            wait_ms = 1;
        }
        if (wait_ms > 10) {
            wait_ms = 10;
        }
    }

    delay(wait_ms);

}
