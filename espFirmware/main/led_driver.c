#include "led_driver.h"

#include "driver/ledc.h"

#define PWM_MAX_DUTY ((1 << 13) - 1)

static const char *LOG_TAG = "led_driver";

typedef struct
{
    int gpio;
    ledc_mode_t speed_mode;
    ledc_channel_t channel;
} channel_cfg_t;

// GPIO 19 = reserved for status LED
static const channel_cfg_t channels[CHANNEL_CNT] = {
    {18, LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0},
    {21, LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1},
    {22, LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_2},
    {23, LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_3},
    {25, LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_4},
    {26, LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_5},
    {27, LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_6},
    {32, LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_7},
    {33, LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0},
};

static uint32_t toDuty(int value)
{
    return (uint32_t)value * PWM_MAX_DUTY / 100;
}

esp_err_t init_led_driver(void)
{
    // Initialize ledc (both high and low speed modes)
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_13_BIT,
        .freq_hz = 5000,
        .timer_num = LEDC_TIMER_0,
        .clk_cfg = LEDC_AUTO_CLK,
    };

    ledc_timer.speed_mode = LEDC_HIGH_SPEED_MODE;
    ESP_RETURN_ON_ERROR(ledc_timer_config(&ledc_timer), LOG_TAG, "ledc_timer_config(high) failed");

    ledc_timer.speed_mode = LEDC_LOW_SPEED_MODE;
    ESP_RETURN_ON_ERROR(ledc_timer_config(&ledc_timer), LOG_TAG, "ledc_timer_config(low) failed");

    for (int i = 0; i < CHANNEL_CNT; i++)
    {
        ledc_channel_config_t ledc_channel = {
            .speed_mode = channels[i].speed_mode,
            .channel = channels[i].channel,
            .timer_sel = LEDC_TIMER_0,
            .intr_type = LEDC_INTR_DISABLE,
            .gpio_num = channels[i].gpio,
            .duty = 0,
            .hpoint = 0};

        ESP_RETURN_ON_ERROR(ledc_channel_config(&ledc_channel), LOG_TAG, "ledc_channel_config(%d) failed", i);
    }

    ESP_RETURN_ON_ERROR(ledc_fade_func_install(0), LOG_TAG, "ledc_fade_func_install failed");

    return ESP_OK;
}

esp_err_t set_channel_value(int channel, int value)
{
    if (channel < 0 || channel >= CHANNEL_CNT || value < 0 || value > 100)
    {
        return ESP_ERR_INVALID_ARG;
    }

    const channel_cfg_t *out_chan = &channels[channel];
    ESP_RETURN_ON_ERROR(ledc_set_duty_and_update(out_chan->speed_mode, out_chan->channel, toDuty(value), 0), LOG_TAG, "set_duty(%d) failed", channel);

    return ESP_OK;
}