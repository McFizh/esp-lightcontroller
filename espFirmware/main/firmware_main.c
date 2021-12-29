#include <stdio.h>

#include "freertos/FreeRTOS.h"

#include "nvs_flash.h"
#include "freertos/event_groups.h"
#include "driver/ledc.h"

#include "connection.h"
#include "tcp_server.h"

#define GOT_IPV4_BIT BIT(0)
#define GOT_IPV6_BIT BIT(1)
#define CONNECTED_BITS (GOT_IPV4_BIT)

void app_main()
{
  ESP_ERROR_CHECK( nvs_flash_init() );

  network_connect();

  // Wait for network connectivity
  xEventGroupWaitBits(connection_event_group, CONNECTED_BITS, false, true, portMAX_DELAY);

  // Initialize ledc
  ledc_timer_config_t ledc_timer = {
      .duty_resolution = LEDC_TIMER_13_BIT,
      .freq_hz = 5000,
      .speed_mode = LEDC_HIGH_SPEED_MODE,
      .timer_num = LEDC_TIMER_0,
      .clk_cfg = LEDC_AUTO_CLK,
  };
  ledc_timer_config(&ledc_timer);

  /*
  ledc_timer.speed_mode = LEDC_HIGH_SPEED_MODE;
  ledc_timer.timer_num = LEDC_TIMER_1;
  ledc_timer_config(&ledc_timer);
  */

  ledc_channel_config_t ledc_channel[1] = {
      {
          .speed_mode = LEDC_HIGH_SPEED_MODE,
          .channel    = LEDC_CHANNEL_0,
          .timer_sel  = LEDC_TIMER_0,
          .intr_type  = LEDC_INTR_DISABLE,
          .gpio_num   = 18,
          .duty       = 4095,       // 50% duty: (2^13 - 1) * 0.5)
          .hpoint     = 0
      }
  };

  ledc_channel_config(&ledc_channel[0]);

  // Init TCP server
  init_tcp_server();
  run_tcp_loop();
}
