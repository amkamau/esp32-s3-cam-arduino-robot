#include "nvs_flash.h"
#include "wifi.h"
#include "camera.h"
#include "web_app.h"
#include "esp_log.h"
#include "uart_stream.h"


#define TAG "main"

#define WIFI_SSID     "NSPW12"
#define WIFI_PASSWORD "Infinity@!1"



void app_main(void){

    /* 1. NVS*/
    ESP_ERROR_CHECK(nvs_flash_init());

    /* 2. Wifi */
    ESP_ERROR_CHECK(wifi_init());
    ESP_ERROR_CHECK(wifi_start_sta(WIFI_SSID, WIFI_PASSWORD));

    /* 3. Camera */
    ESP_ERROR_CHECK(camera_init());

    /* 4. UART */
    ESP_ERROR_CHECK(uart_init());    
    ESP_ERROR_CHECK(uart_start_task());
    
    /* 5. Web */
    ESP_ERROR_CHECK(web_app_start());

}