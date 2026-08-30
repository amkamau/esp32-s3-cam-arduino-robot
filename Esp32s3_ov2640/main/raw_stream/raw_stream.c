#include "raw_stream.h"

#include "esp_camera.h"
#include "esp_log.h"

#include <string.h>
#include <errno.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define TAG "raw_stream"

#define SERVER_PORT 8080
#define BACKLOG     1

static esp_err_t send_all(int sock, const uint8_t *data, size_t len)
{
    while (len > 0) {
        int sent = send(sock, data, len, 0);

        if (sent < 0) {
            ESP_LOGW(TAG, "send failed: errno=%d", errno);
            return ESP_FAIL;
        }

        if (sent == 0) {
            return ESP_FAIL;
        }

        data += sent;
        len -= sent;
    }

    return ESP_OK;
}

static void raw_stream_task(void *arg)
{
    int server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

    if (server_sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket");
        vTaskDelete(NULL);
        return;
    }

    int reuse = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR,
               &reuse, sizeof(reuse));

    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(SERVER_PORT),
        .sin_addr.s_addr = htonl(INADDR_ANY),
    };

    if (bind(server_sock,
             (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0) {

        ESP_LOGE(TAG, "bind failed: errno=%d", errno);
        close(server_sock);
        vTaskDelete(NULL);
        return;
    }

    if (listen(server_sock, BACKLOG) < 0) {
        ESP_LOGE(TAG, "listen failed: errno=%d", errno);
        close(server_sock);
        vTaskDelete(NULL);
        return;
    }

    ESP_LOGI(TAG, "Raw camera server listening on port %d", SERVER_PORT);

    while (1) {

        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        int client_sock = accept(
            server_sock,
            (struct sockaddr *)&client_addr,
            &client_len
        );

        if (client_sock < 0) {
            ESP_LOGW(TAG, "accept failed: errno=%d", errno);
            continue;
        }

        ESP_LOGI(TAG, "PC connected");

        while (1) {

            camera_fb_t *fb = esp_camera_fb_get();

            if (!fb) {
                ESP_LOGW(TAG, "camera frame failed");
                continue;
            }

            /*
             * VGA RGB565:
             *
             * 640 * 480 * 2 = 614400 bytes
             */
            esp_err_t err = send_all(
                client_sock,
                fb->buf,
                fb->len
            );

            size_t frame_len = fb->len;

            esp_camera_fb_return(fb);

            if (err != ESP_OK) {
                ESP_LOGW(TAG, "PC disconnected");
                break;
            }

            ESP_LOGD(TAG, "sent frame: %u bytes",
                     (unsigned)frame_len);
        }

        shutdown(client_sock, SHUT_RDWR);
        close(client_sock);

        ESP_LOGI(TAG, "Client connection closed");
    }
}

esp_err_t raw_stream_start(int port)
{
    (void)port;

    xTaskCreate(
        raw_stream_task,
        "raw_stream",
        8192,
        NULL,
        5,
        NULL
    );

    return ESP_OK;
}
    