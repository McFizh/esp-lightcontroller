#include "connection.h"
#include "tcp_server.h"
#include "led_driver.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include "nvs_flash.h"
#include "driver/gpio.h"

#define STATUS_LED_GPIO GPIO_NUM_19

static void error_loop(void)
{
    while (1)
    {
        gpio_set_level(STATUS_LED_GPIO, 1);
        vTaskDelay(pdMS_TO_TICKS(500));
        gpio_set_level(STATUS_LED_GPIO, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void app_main(void)
{
    // Initialize status led & led driver
    gpio_reset_pin(STATUS_LED_GPIO);
    gpio_set_direction(STATUS_LED_GPIO, GPIO_MODE_OUTPUT);

    if (init_led_driver() != ESP_OK)
        error_loop();

    // Start network, and enter fail loop, if net init fails (board is useless without network)
    ESP_ERROR_CHECK(nvs_flash_init());
    if (network_connect() != ESP_OK)
        error_loop();

    // Init TCP server & start tcp task
    if (init_tcp_server() != ESP_OK)
        error_loop();

    run_tcp_loop();
}
