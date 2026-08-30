#include "wifi.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include <string.h>
#include "lwip/ip4_addr.h"

#define TAG             "wifi"
#define CONNECTED_BIT   BIT0
#define FAIL_BIT        BIT1
#define MAX_RETRIES     5

static EventGroupHandle_t s_event_group;
static int s_retry_count = 0;
static esp_netif_t *s_sta_netif;

static void sta_event_handler(void *arg, esp_event_base_t base, int32_t id, void *data) {
    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_count < MAX_RETRIES) {
            esp_wifi_connect();
            s_retry_count++;
            ESP_LOGI(TAG, "retry %d/%d", s_retry_count, MAX_RETRIES);
        } else {
            xEventGroupSetBits(s_event_group, FAIL_BIT);
        }
    } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)data;
        ESP_LOGI(TAG, "connected, ip: " IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_count = 0;
        xEventGroupSetBits(s_event_group, CONNECTED_BIT);
    }
}

esp_err_t wifi_init(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    s_sta_netif = esp_netif_create_default_wifi_sta();
    if (s_sta_netif == NULL) return ESP_ERR_NO_MEM;

    s_event_group = xEventGroupCreate();
    if (s_event_group == NULL) {
        return ESP_ERR_NO_MEM;
    }

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    return esp_wifi_init(&cfg);
}


esp_err_t wifi_start_sta(const char *ssid, const char *password) {
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, sta_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, sta_event_handler, NULL);

    wifi_config_t cfg = {};
    strncpy((char *)cfg.sta.ssid, ssid, sizeof(cfg.sta.ssid) - 1);
    strncpy((char *)cfg.sta.password, password, sizeof(cfg.sta.password) - 1);

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &cfg);

    esp_netif_ip_info_t ip_info;
    IP4_ADDR(&ip_info.ip, 192, 168, 0, 50);
    IP4_ADDR(&ip_info.gw, 192, 168, 0, 1);
    IP4_ADDR(&ip_info.netmask, 255, 255, 255, 0);
    ESP_ERROR_CHECK(esp_netif_dhcpc_stop(s_sta_netif));
    ESP_ERROR_CHECK(esp_netif_set_ip_info(s_sta_netif, &ip_info));

    esp_wifi_start();

        ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));

    EventBits_t bits = xEventGroupWaitBits(s_event_group,
        CONNECTED_BIT | FAIL_BIT, pdFALSE, pdFALSE, pdMS_TO_TICKS(15000));

    if (bits & CONNECTED_BIT) return ESP_OK;
    ESP_LOGW(TAG, "failed to connect to %s", ssid);
    return ESP_FAIL;
}
