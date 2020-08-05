#include <stdio.h>

#include "freertos/FreeRTOS.h"

#include "nvs_flash.h"
#include "freertos/event_groups.h"

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

  // Init TCP server
  init_tcp_server();
  run_tcp_loop();
}
