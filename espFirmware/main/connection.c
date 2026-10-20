#include "connection.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include "esp_event.h"
#include "esp_wifi.h"
#include "mdns.h"

#include "../build/main/app_config.h"

#define CONNECTED_BIT BIT0
#define FAIL_BIT BIT1

static const char *LOG_TAG = "wifi";
static EventGroupHandle_t connection_event_group;

static void on_got_ip(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    xEventGroupSetBits(connection_event_group, CONNECTED_BIT);

    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    ESP_LOGI(LOG_TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
}

static void on_wifi_event(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        ESP_LOGE(LOG_TAG, "Disconnected");

        xEventGroupClearBits(connection_event_group, CONNECTED_BIT);
        xEventGroupSetBits(connection_event_group, FAIL_BIT);

        esp_err_t err = esp_wifi_connect();
        if (err == ESP_ERR_WIFI_NOT_STARTED)
        {
            return;
        }
        ESP_ERROR_CHECK(err);
    }
}

esp_err_t network_connect(void)
{
    connection_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_t *netif = esp_netif_create_default_wifi_sta();
    assert(netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &on_wifi_event, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &on_got_ip, NULL));

    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = wifi_ssid,
            .password = wifi_key,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

    ESP_ERROR_CHECK(esp_wifi_start());

    // Wait for connection or failure
    EventBits_t eventBits =
        xEventGroupWaitBits(connection_event_group, CONNECTED_BIT | FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);

    if (eventBits & CONNECTED_BIT)
    {
        // Setup MDNS broadcast
        ESP_RETURN_ON_ERROR(mdns_init(), LOG_TAG, "mdns init failed");
        ESP_RETURN_ON_ERROR(mdns_hostname_set("esp-fw"), LOG_TAG, "mdns hostname set failed");
        ESP_RETURN_ON_ERROR(mdns_instance_name_set("Lightshow ESP firmware"), LOG_TAG, "instance name set failed");
        ESP_RETURN_ON_ERROR(mdns_service_add("ESP lightshow", "_lightctrl", "_tcp", SERVER_PORT, NULL, 0), LOG_TAG, "mdns service set failed");
        return ESP_OK;
    }
    else
    {
        return ESP_FAIL;
    }
}
