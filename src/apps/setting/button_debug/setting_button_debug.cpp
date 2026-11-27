#include "setting_button_debug.h"

#include "../../../core/input/button_service.h"
#include "../../../../include/input_pins.h"
#include "../../../ui/assets/include_theme.h"

namespace {
constexpr size_t BUTTON_COUNT = 4;
constexpr uint32_t BUTTON_PRESSED_COLOR = 0x40A878;
constexpr uint32_t BUTTON_RELEASED_COLOR = 0x718096;

struct ButtonRow {
    const char *name;
    uint8_t pcf_pin;
    lv_obj_t *state_label;
};

ButtonRow button_rows[BUTTON_COUNT] = {
    {"BACK (P17)", BUTTON_BACK_PCF_BIT, nullptr},
    {"HOME (P16)", BUTTON_HOME_PCF_BIT, nullptr},
    {"OPTION (P15)", BUTTON_OPTION_PCF_BIT, nullptr},
    {"FAVORITE (P14)", BUTTON_FAVORITE_PCF_BIT, nullptr},
};
lv_obj_t *connection_label;
lv_timer_t *refresh_timer;

void refreshButtonStates(lv_timer_t *timer)
{
    (void)timer;
    const bool available = button_service_pcf8575_available();
    const uint16_t pressed_mask = button_service_pcf8575_pressed_mask();
    if (connection_label != nullptr) {
        lv_label_set_text(connection_label,
                          available ? "PCF8575 connected at 0x20" :
                                      "PCF8575 unavailable; check I2C wiring");
        lv_obj_set_style_text_color(connection_label,
                                    available ? lv_color_hex(BUTTON_PRESSED_COLOR) :
                                                COLOR_HINT_GRAY,
                                    LV_PART_MAIN);
    }

    for (size_t i = 0; i < BUTTON_COUNT; ++i) {
        lv_obj_t *label = button_rows[i].state_label;
        if (label == nullptr) continue;
        if (!available) {
            lv_label_set_text(label, "--");
            lv_obj_set_style_text_color(label, COLOR_HINT_GRAY, LV_PART_MAIN);
            continue;
        }

        const bool pressed = (pressed_mask & (1U << button_rows[i].pcf_pin)) != 0;
        lv_label_set_text(label, pressed ? "PRESSED" : "Released");
        lv_obj_set_style_text_color(label,
                                    lv_color_hex(pressed ? BUTTON_PRESSED_COLOR :
                                                          BUTTON_RELEASED_COLOR),
                                    LV_PART_MAIN);
    }
}

void createButtonRow(lv_obj_t *parent, size_t index, lv_coord_t y)
{
    lv_obj_t *row = lv_obj_create(parent);
    lv_obj_remove_style_all(row);
    lv_obj_set_size(row, lv_pct(92), 30);
    lv_obj_align(row, LV_ALIGN_TOP_MID, 0, y);
    lv_obj_set_style_bg_color(row, COLOR_BACKGROUND, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(row, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(row, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(row, lv_color_hex(0xD5DCE5), LV_PART_MAIN);
    lv_obj_set_style_radius(row, 6, LV_PART_MAIN);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *name = lv_label_create(row);
    lv_label_set_text(name, button_rows[index].name);
    lv_obj_set_style_text_color(name, COLOR_MAIN_CONTENT, LV_PART_MAIN);
    lv_obj_set_style_text_font(name, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_obj_align(name, LV_ALIGN_LEFT_MID, 8, 0);

    button_rows[index].state_label = lv_label_create(row);
    lv_obj_set_style_text_font(button_rows[index].state_label,
                               &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_align(button_rows[index].state_label, LV_ALIGN_RIGHT_MID, -8, 0);
}
} // namespace

extern "C" void setting_button_debug_create_ui(lv_obj_t *parent)
{
    lv_obj_t *title = lv_label_create(parent);
    lv_label_set_text(title, "PCF8575 Button Debug");
    lv_obj_set_style_text_color(title, COLOR_MAIN_CONTENT, LV_PART_MAIN);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, LV_PART_MAIN);
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 12, 6);

    connection_label = lv_label_create(parent);
    lv_obj_set_style_text_font(connection_label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_align(connection_label, LV_ALIGN_TOP_LEFT, 12, 35);

    for (size_t i = 0; i < BUTTON_COUNT; ++i) {
        createButtonRow(parent, i, 62 + static_cast<lv_coord_t>(i) * 35);
    }

    refreshButtonStates(nullptr);
    refresh_timer = lv_timer_create(refreshButtonStates, 100, nullptr);
}

extern "C" void setting_button_debug_close_ui(void)
{
    if (refresh_timer != nullptr) {
        lv_timer_del(refresh_timer);
        refresh_timer = nullptr;
    }
    connection_label = nullptr;
    for (size_t i = 0; i < BUTTON_COUNT; ++i) {
        button_rows[i].state_label = nullptr;
    }
}
