#pragma once

#include "esp_check.h"

#define CHANNEL_CNT 9

esp_err_t init_led_driver(void);
esp_err_t set_channel_value(int channel, int value);