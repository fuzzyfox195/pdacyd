#pragma once

#include <stdint.h>

#define PCF8575_I2C_ADDRESS       0x20
/* PCF8575 port labels map to register bits: P17..P10 are bits 15..8. */
#define BUTTON_BACK_PCF_BIT       15  /* P17 */
#define BUTTON_HOME_PCF_BIT       14  /* P16 */
#define BUTTON_OPTION_PCF_BIT     13  /* P15 */
#define BUTTON_FAVORITE_PCF_BIT   12  /* P14 */
#define SLEEP_BUTTON_GPIO         14
#define BUTTON_ACTIVE_LOW         1

#define BUTTON_DEBOUNCE_MS        35U
#define BUTTON_POLL_INTERVAL_MS   20U
#define SLEEP_LONG_PRESS_MS       2500U
#define SLEEP_RELEASE_TIMEOUT_MS  10000U
