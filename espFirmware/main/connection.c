#include <string.h>
#include <stdio.h>

#include "freertos/FreeRTOS.h"

#include "freertos/event_groups.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "mdns.h"

#include "../build/main/app_config.h"

#define GOT_IPV4_BIT BIT(0)
#define GOT_IPV6_BIT BIT(1)

EventGroupHandle_t connection_event_group;

static void on_got_ip(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    printf("Got IP!\n");
    xEventGroupSetBits(connection_event_group, GOT_IPV4_BIT);
}

static void on_wifi_disconnect(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    printf("Disconnected!\n");
    xEventGroupClearBits(connection_event_group, GOT_IPV4_BIT);
    esp_err_t err = esp_wifi_connect();
    if (err == ESP_ERR_WIFI_NOT_STARTED) {
        return;
    }
    ESP_ERROR_CHECK(err);
}

void network_connect() {
    connection_event_group = xEventGroupCreate();

    esp_netif_init();
    esp_event_loop_create_default();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_netif_config_t netif_config = ESP_NETIF_DEFAULT_WIFI_STA();
    esp_netif_t *netif = esp_netif_new(&netif_config);

    assert(netif);

    esp_netif_attach_wifi_station(netif);
    esp_wifi_set_default_wifi_sta_handlers();

    esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &on_wifi_disconnect, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &on_got_ip, NULL);

    esp_wifi_set_storage(WIFI_STORAGE_RAM);
    esp_wifi_set_mode(WIFI_MODE_STA);

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = wifi_ssid,
            .password = wifi_key,
        },
    };
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);

    esp_wifi_start();
    esp_wifi_connect();

    // Setup MDNS broadcast
    ESP_ERROR_CHECK( mdns_init() );
    ESP_ERROR_CHECK( mdns_hostname_set("esp-fw") );
    ESP_ERROR_CHECK( mdns_instance_name_set("Lightshow ESP firmware") );
    ESP_ERROR_CHECK( mdns_service_add("ESP lightshow", "_lightctrl", "_tcp", 5900, NULL, 0) );
}
