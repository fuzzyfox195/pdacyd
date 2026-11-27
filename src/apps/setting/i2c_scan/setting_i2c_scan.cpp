#include "setting_i2c_scan.h"

#include "../../../core/storage/sd_storage.h"
#include "../../../core/storage/storage_config.h"
#include "../../../ui/assets/include_theme.h"
#include "../../../../include/lcd_pins.h"

#include <Arduino.h>
#include <LovyanGFX.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

namespace {
constexpr size_t MAX_I2C_DEVICES = 120;
constexpr uint8_t FIRST_I2C_ADDRESS = 0x03;
constexpr uint8_t LAST_I2C_ADDRESS = 0x77;
constexpr lv_coord_t HEADER_HEIGHT = 52;
constexpr lv_coord_t TABLE_ROW_HEIGHT = 27;
constexpr uint32_t SCAN_INTERVAL_MS = 8;

struct KnownDevice {
    char address[8];
    char part[32];
};

static lv_obj_t *known_list;
static lv_obj_t *unknown_list;
static lv_obj_t *scan_spinner;
static lv_obj_t *scan_status;
static lv_timer_t *scan_timer;
static KnownDevice known_devices[MAX_I2C_DEVICES];
static uint8_t detected_addresses[MAX_I2C_DEVICES];
static uint8_t unknown_addresses[MAX_I2C_DEVICES];
static size_t known_count;
static size_t detected_count;
static size_t unknown_count;
static uint8_t scan_address;
static bool database_loaded;

bool copy_json_string(const char *object, const char *object_end,
                      const char *key, char *out, size_t out_size)
{
    char key_token[24];
    snprintf(key_token, sizeof(key_token), "\"%s\"", key);
    const char *key_pos = strstr(object, key_token);
    if (key_pos == nullptr || key_pos >= object_end) return false;

    const char *value = strchr(key_pos + strlen(key_token), ':');
    if (value == nullptr || value >= object_end) return false;
    ++value;
    while (value < object_end && (*value == ' ' || *value == '\t' ||
                                  *value == '\r' || *value == '\n')) {
        ++value;
    }
    if (value >= object_end || *value != '"') return false;
    ++value;

    size_t written = 0;
    while (value < object_end && *value != '"') {
        if (*value == '\\' && value + 1 < object_end) ++value;
        if (written + 1 < out_size) out[written++] = *value;
        ++value;
    }
    if (value >= object_end || out_size == 0) return false;
    out[written] = '\0';
    return true;
}

void load_known_devices(void)
{
    known_count = 0;
    database_loaded = false;
    if (!SDStorage::isMounted()) return;

    String json;
    if (!SDStorage::readJson(SDStorage::I2C_DATA, json)) return;

    const char *cursor = strstr(json.c_str(), "\"i2c\"");
    if (cursor == nullptr) return;
    cursor = strchr(cursor, '[');
    if (cursor == nullptr) return;
    ++cursor;
    database_loaded = true;

    while (*cursor != '\0' && *cursor != ']' && known_count < MAX_I2C_DEVICES) {
        if (*cursor != '{') {
            ++cursor;
            continue;
        }
        const char *object_end = strchr(cursor, '}');
        if (object_end == nullptr) break;

        KnownDevice device{};
        if (copy_json_string(cursor, object_end, "address",
                             device.address, sizeof(device.address)) &&
            copy_json_string(cursor, object_end, "part",
                             device.part, sizeof(device.part))) {
            known_devices[known_count++] = device;
        }
        cursor = object_end + 1;
    }
}

void set_label(lv_obj_t *label, const char *text)
{
    if (label != nullptr) lv_label_set_text(label, text);
}

void clear_scan_widgets(void)
{
    if (scan_timer != nullptr) {
        lv_timer_del(scan_timer);
        scan_timer = nullptr;
    }
    if (scan_spinner != nullptr) {
        lv_obj_del(scan_spinner);
        scan_spinner = nullptr;
    }
    if (scan_status != nullptr) {
        lv_obj_del(scan_status);
        scan_status = nullptr;
    }
}

int find_known_device(uint8_t address)
{
    char scanned[8];
    snprintf(scanned, sizeof(scanned), "0x%02X", address);
    for (size_t i = 0; i < known_count; ++i) {
        if (strcasecmp(known_devices[i].address, scanned) == 0) return (int)i;
    }
    return -1;
}

bool probe_address(uint8_t address)
{
    auto transaction = lgfx::i2c::beginTransaction(
        TOUCH_I2C_PORT, address, TOUCH_I2C_FREQ, false);
    if (transaction.has_error()) return false;

    return lgfx::i2c::endTransaction(TOUCH_I2C_PORT).has_value();
}

void create_row(lv_obj_t *list, lv_coord_t width, lv_coord_t y,
                const char *left_text, const char *right_text,
                bool known_row)
{
    lv_obj_t *address = lv_label_create(list);
    lv_label_set_text(address, left_text);
    lv_obj_set_width(address, known_row ? width / 2 - 4 : width - 4);
    lv_obj_set_style_text_color(address, COLOR_MAIN_CONTENT, LV_PART_MAIN);
    lv_obj_set_style_text_font(address, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_obj_align(address, LV_ALIGN_TOP_LEFT, 2, y);

    if (known_row) {
        lv_obj_t *part = lv_label_create(list);
        lv_label_set_text(part, right_text);
        lv_obj_set_width(part, width / 2 - 4);
        lv_label_set_long_mode(part, LV_LABEL_LONG_DOT);
        lv_obj_set_style_text_color(part, COLOR_MAIN_CONTENT, LV_PART_MAIN);
        lv_obj_set_style_text_font(part, &lv_font_montserrat_14, LV_PART_MAIN);
        lv_obj_align(part, LV_ALIGN_TOP_LEFT, width / 2, y);
    }
}

void render_results(void)
{
    if (known_list == nullptr || unknown_list == nullptr) return;
    lv_obj_clean(known_list);
    lv_obj_clean(unknown_list);

    const lv_coord_t known_width = lv_obj_get_width(known_list);
    const lv_coord_t unknown_width = lv_obj_get_width(unknown_list);
    size_t known_row = 0;
    for (size_t i = 0; i < known_count; ++i) {
        const uint8_t address = (uint8_t)strtol(known_devices[i].address, nullptr, 0);
        bool detected = false;
        for (size_t j = 0; j < detected_count; ++j) {
            if (detected_addresses[j] == address) {
                detected = true;
                break;
            }
        }
        if (!detected) continue;

        char address_text[8];
        snprintf(address_text, sizeof(address_text), "0x%02X", address);
        create_row(known_list, known_width,
                   (lv_coord_t)known_row++ * TABLE_ROW_HEIGHT,
                   address_text, known_devices[i].part, true);
    }

    for (size_t i = 0; i < unknown_count; ++i) {
        char address_text[8];
        snprintf(address_text, sizeof(address_text), "0x%02X", unknown_addresses[i]);
        create_row(unknown_list, unknown_width,
                   (lv_coord_t)i * TABLE_ROW_HEIGHT,
                   address_text, nullptr, false);
    }

    if (!database_loaded) {
        lv_obj_t *warning = lv_label_create(known_list);
        lv_label_set_text(warning, "i2c.json unavailable");
        lv_obj_set_style_text_color(warning, COLOR_HINT_GRAY, LV_PART_MAIN);
        lv_obj_set_style_text_font(warning, CONTENT_FONT, LV_PART_MAIN);
        lv_obj_align(warning, LV_ALIGN_TOP_LEFT, 2,
                     (lv_coord_t)known_row * TABLE_ROW_HEIGHT);
    }
}

void finish_scan(void)
{
    if (scan_timer != nullptr) {
        lv_timer_del(scan_timer);
        scan_timer = nullptr;
    }
    if (scan_spinner != nullptr) {
        lv_obj_del(scan_spinner);
        scan_spinner = nullptr;
    }
    if (scan_status != nullptr) {
        lv_obj_del(scan_status);
        scan_status = nullptr;
    }
    render_results();
}

void scan_timer_cb(lv_timer_t *timer)
{
    (void)timer;
    if (scan_address > LAST_I2C_ADDRESS) {
        finish_scan();
        return;
    }

    const uint8_t address = scan_address++;
    if (!probe_address(address)) return;

    if (detected_count < MAX_I2C_DEVICES) {
        detected_addresses[detected_count++] = address;
    }
    if (find_known_device(address) >= 0) return;
    if (unknown_count < MAX_I2C_DEVICES) unknown_addresses[unknown_count++] = address;
}

lv_obj_t *create_scroll_list(lv_obj_t *parent, lv_coord_t x, lv_coord_t y,
                             lv_coord_t width, lv_coord_t height)
{
    lv_obj_t *list = lv_obj_create(parent);
    lv_obj_remove_style_all(list);
    lv_obj_set_pos(list, x, y);
    lv_obj_set_size(list, width, height);
    lv_obj_set_scroll_dir(list, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(list, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_opa(list, LV_OPA_TRANSP, LV_PART_MAIN);
    return list;
}

void create_header(lv_obj_t *parent, const char *text, lv_coord_t x,
                   lv_coord_t y, lv_coord_t width)
{
    lv_obj_t *header = lv_obj_create(parent);
    lv_obj_remove_style_all(header);
    lv_obj_set_pos(header, x, y);
    lv_obj_set_size(header, width, 23);
    lv_obj_set_style_bg_color(header, lv_color_hex(0x008A3B), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(header, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(header, 12, LV_PART_MAIN);

    lv_obj_t *label = lv_label_create(header);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_obj_center(label);
}
}

extern "C" void setting_i2c_scan_create_ui(lv_obj_t *parent)
{
    clear_scan_widgets();
    known_count = 0;
    detected_count = 0;
    unknown_count = 0;
    scan_address = FIRST_I2C_ADDRESS;
    known_list = nullptr;
    unknown_list = nullptr;
    load_known_devices();

    const lv_coord_t width = lv_obj_get_width(parent);
    const lv_coord_t height = lv_obj_get_height(parent);
    const lv_coord_t divider_x = width * 3 / 5;

    lv_obj_t *title = lv_label_create(parent);
    lv_label_set_text(title, "I2C");
    lv_obj_set_style_text_color(title, COLOR_MAIN_CONTENT, LV_PART_MAIN);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_22, LV_PART_MAIN);
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 8, 4);

    lv_obj_t *unknown_title = lv_label_create(parent);
    lv_label_set_text(unknown_title, "Unknown");
    lv_obj_set_style_text_color(unknown_title, COLOR_MAIN_CONTENT, LV_PART_MAIN);
    lv_obj_set_style_text_font(unknown_title, &lv_font_montserrat_22, LV_PART_MAIN);
    lv_obj_align(unknown_title, LV_ALIGN_TOP_LEFT, divider_x + 8, 4);

    lv_obj_t *divider = lv_obj_create(parent);
    lv_obj_remove_style_all(divider);
    lv_obj_set_pos(divider, divider_x, 0);
    lv_obj_set_size(divider, 2, height);
    lv_obj_set_style_bg_color(divider, COLOR_HINT_GRAY, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(divider, LV_OPA_COVER, LV_PART_MAIN);

    const lv_coord_t left_width = divider_x - 12;
    create_header(parent, "Address", 4, 32, left_width / 2 - 3);
    create_header(parent, "Part code", left_width / 2 + 8, 32,
                  left_width / 2 - 8);

    known_list = create_scroll_list(parent, 8, HEADER_HEIGHT + 8,
                                    left_width, height - HEADER_HEIGHT - 4);
    unknown_list = create_scroll_list(parent, divider_x + 8, 32,
                                      width - divider_x - 8, height - 40);

    scan_status = lv_label_create(parent);
    lv_label_set_text(scan_status, "Scanning...");
    lv_obj_set_style_text_color(scan_status, COLOR_HINT_GRAY, LV_PART_MAIN);
    lv_obj_set_style_text_font(scan_status, CONTENT_FONT, LV_PART_MAIN);
    lv_obj_align(scan_status, LV_ALIGN_TOP_LEFT, 6, HEADER_HEIGHT + 18);

    scan_spinner = lv_spinner_create(parent, 700, 60);
    lv_obj_set_size(scan_spinner, 22, 22);
    lv_obj_align(scan_spinner, LV_ALIGN_TOP_LEFT, 88, HEADER_HEIGHT +16);

    const auto bus_init = lgfx::i2c::init(TOUCH_I2C_PORT, TOUCH_SDA, TOUCH_SCL);
    if (bus_init.has_error()) {
        set_label(scan_status, "I2C bus unavailable");
        if (scan_spinner != nullptr) {
            lv_obj_del(scan_spinner);
            scan_spinner = nullptr;
        }
        render_results();
        return;
    }
    scan_timer = lv_timer_create(scan_timer_cb, SCAN_INTERVAL_MS, nullptr);
}

extern "C" void setting_i2c_scan_close_ui(void)
{
    clear_scan_widgets();
    known_list = nullptr;
    unknown_list = nullptr;
}
