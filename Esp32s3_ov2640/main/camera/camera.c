#include "camera.h"
#include "esp_camera.h"
#include "esp_log.h"
#include <stdio.h>

#define TAG "camera"

#define PWDN_GPIO_NUM   -1
#define RESET_GPIO_NUM  -1
#define XCLK_GPIO_NUM   15
#define SIOD_GPIO_NUM    4
#define SIOC_GPIO_NUM    5
#define Y9_GPIO_NUM     16
#define Y8_GPIO_NUM     17
#define Y7_GPIO_NUM     18
#define Y6_GPIO_NUM     12
#define Y5_GPIO_NUM     10
#define Y4_GPIO_NUM      8
#define Y3_GPIO_NUM      9
#define Y2_GPIO_NUM     11
#define VSYNC_GPIO_NUM   6
#define HREF_GPIO_NUM    7
#define PCLK_GPIO_NUM   13

esp_err_t camera_init(void) {
    camera_config_t config = {
        .pin_pwdn      = PWDN_GPIO_NUM,
        .pin_reset     = RESET_GPIO_NUM,
        .pin_xclk      = XCLK_GPIO_NUM,
        .pin_sccb_sda  = SIOD_GPIO_NUM,
        .pin_sccb_scl  = SIOC_GPIO_NUM,
        .pin_d7        = Y9_GPIO_NUM,
        .pin_d6        = Y8_GPIO_NUM,
        .pin_d5        = Y7_GPIO_NUM,
        .pin_d4        = Y6_GPIO_NUM,
        .pin_d3        = Y5_GPIO_NUM,
        .pin_d2        = Y4_GPIO_NUM,
        .pin_d1        = Y3_GPIO_NUM,
        .pin_d0        = Y2_GPIO_NUM,
        .pin_vsync     = VSYNC_GPIO_NUM,
        .pin_href      = HREF_GPIO_NUM,
        .pin_pclk      = PCLK_GPIO_NUM,
        .xclk_freq_hz  = 20000000,
        .ledc_timer    = LEDC_TIMER_0,
        .ledc_channel  = LEDC_CHANNEL_0,
        .pixel_format  = PIXFORMAT_JPEG,
        .frame_size    = FRAMESIZE_VGA,
        .jpeg_quality  = 15,
        .fb_count      = 2,
        .fb_location   = CAMERA_FB_IN_PSRAM,
        .grab_mode     = CAMERA_GRAB_LATEST,
    };

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "init failed: %s", esp_err_to_name(err));
        return err;
    }

    ESP_LOGI(TAG, "init OK");
    return ESP_OK;
}


esp_err_t handle_camera_stream(httpd_req_t *req) {
    httpd_resp_set_type(req, "multipart/x-mixed-replace;boundary=frame");
    httpd_resp_set_hdr(req, "Cache-Control", "no-store");

    char part_hdr[64];
    esp_err_t res = ESP_OK;

    while (res == ESP_OK) {
        camera_fb_t *fb = esp_camera_fb_get();

        if (!fb) {
            ESP_LOGW(TAG, "frame capture failed");
            continue;
        }

        int hdr_len = snprintf(
            part_hdr,
            sizeof(part_hdr),
            "--frame\r\n"
            "Content-Type: image/jpeg\r\n"
            "Content-Length: %u\r\n\r\n",
            (unsigned)fb->len
        );

        res = httpd_resp_send_chunk(req, part_hdr, hdr_len);

        if (res == ESP_OK) {
            res = httpd_resp_send_chunk(
                req,
                (const char *)fb->buf,
                fb->len
            );
        }

        if (res == ESP_OK) {
            res = httpd_resp_send_chunk(req, "\r\n", 2);
        }

        esp_camera_fb_return(fb);

        vTaskDelay(pdMS_TO_TICKS(100));
    }

    return ESP_OK;
}

