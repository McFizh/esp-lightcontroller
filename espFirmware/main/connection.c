#include <string.h>
#include <stdio.h>

#include "freertos/FreeRTOS.h"

#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "lwip/ip_addr.h"
#include "freertos/event_groups.h"
#include "mdns.h"

#include "app_config.h"

#define GOT_IPV4_BIT BIT(0)
#define GOT_IPV6_BIT BIT(1)

EventGroupHandle_t connection_event_group;

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    /* For accessing reason codes in case of disconnection */
    system_event_info_t *info = &event->event_info;

    switch(event->event_id) {
      case SYSTEM_EVENT_STA_START:
          esp_wifi_connect();
          break;
      case SYSTEM_EVENT_STA_CONNECTED:
          tcpip_adapter_create_ip6_linklocal(TCPIP_ADAPTER_IF_STA);
          break;
      case SYSTEM_EVENT_STA_GOT_IP:
          xEventGroupSetBits(connection_event_group, GOT_IPV4_BIT);
          break;
      case SYSTEM_EVENT_AP_STA_GOT_IP6:
          xEventGroupSetBits(connection_event_group, GOT_IPV6_BIT);
          break;
      case SYSTEM_EVENT_STA_DISCONNECTED:
          printf("Disconnect reason : %d", info->disconnected.reason);

          if (info->disconnected.reason == WIFI_REASON_BASIC_RATE_NOT_SUPPORT) {
              /*Switch to 802.11 bgn mode */
              esp_wifi_set_protocol(ESP_IF_WIFI_STA, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N);
          }

          esp_wifi_connect();
          xEventGroupClearBits(connection_event_group, GOT_IPV4_BIT | GOT_IPV6_BIT);
          break;
      default:
          break;
    }

    mdns_handle_system_event(ctx, event);
    return ESP_OK;
}

void setup_mdns() {
  ESP_ERROR_CHECK( mdns_init() );
  ESP_ERROR_CHECK( mdns_hostname_set("esp-fw") );
  ESP_ERROR_CHECK( mdns_instance_name_set("Lightshow ESP firmware") );
  ESP_ERROR_CHECK( mdns_service_add("ESP lightshow", "_lightctrl", "_tcp", 5900, NULL, 0) );
}

void network_connect() {
  connection_event_group = xEventGroupCreate();
  tcpip_adapter_init();

  ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
  ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );

  wifi_config_t wifi_config = {
      .sta = {
          .ssid = wifi_ssid,
          .password = wifi_key,
      },
  };

  ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
  ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
  ESP_ERROR_CHECK( esp_wifi_start() );

  setup_mdns();
}
