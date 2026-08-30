#include "web_app.h"
#include "http_server.h"
#include "camera.h"
#include "uart_stream.h"
#include "esp_log.h"
#include <string.h>

#define TAG "web_app"

extern const char index_html_start[] asm("_binary_index_html_start");
extern const char index_html_end[]   asm("_binary_index_html_end");

static httpd_handle_t s_server = NULL;


/* ----------------------------------------------------------
   Index
   ---------------------------------------------------------- */

static esp_err_t handle_index(httpd_req_t *req){
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, index_html_start, index_html_end - index_html_start);
    return ESP_OK;
}


/* ----------------------------------------------------------
   Arduino -> PC
   GET /api/robot/state
   ---------------------------------------------------------- */
static esp_err_t handle_robot_state(httpd_req_t *req){
    char state[128];
    esp_err_t err = uart_get_robot_state( state, sizeof(state));
    if (err != ESP_OK) {
        httpd_resp_send_err( req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to get robot state" );
        return err;
    }
    httpd_resp_set_type(req, "text/plain");
    httpd_resp_send(req, state, strlen(state));
    return ESP_OK;
}


static esp_err_t handle_arduino_command(httpd_req_t *req){
    char command[128];
    if (req->content_len == 0) {
        httpd_resp_send_err( req, HTTPD_400_BAD_REQUEST, "Empty command");
        return ESP_FAIL;
    }
    if (req->content_len >= sizeof(command)) {
        httpd_resp_send_err( req, HTTPD_400_BAD_REQUEST, "Command too long");
        return ESP_FAIL;
    }

    size_t total_received = 0;

    while (total_received < req->content_len) {
        int received = httpd_req_recv( req, command + total_received, req->content_len - total_received);
        if (received == HTTPD_SOCK_ERR_TIMEOUT) {
            continue;
        }
        if (received <= 0) {
            httpd_resp_send_err( req, HTTPD_400_BAD_REQUEST, "Failed to receive command" );
            return ESP_FAIL;
        }
        total_received += received;
    }
    command[total_received] = '\0';

    /* Pass complete command to UART module */
    esp_err_t err = uart_send_arduino_command(command);

    if (err != ESP_OK) {
        httpd_resp_send_err( req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send command");
        return err;
    }

    httpd_resp_set_type( req, "text/plain");

    httpd_resp_send(req,"OK",2);

    return ESP_OK;
}


/* ----------------------------------------------------------
   Start web application
   ---------------------------------------------------------- */
esp_err_t web_app_start(void){

    esp_err_t err = http_server_start(&s_server);
    if (err != ESP_OK) return err;

    static const httpd_uri_t routes[] = {
        { .uri = "/", .method = HTTP_GET, .handler = handle_index},
        { .uri = "/api/camera/stream", .method = HTTP_GET, .handler = handle_camera_stream },
        { .uri = "/api/robot/state", .method = HTTP_GET, .handler = handle_robot_state},
        { .uri = "/api/arduino/command", .method = HTTP_POST, .handler = handle_arduino_command}
    };

    for (int i = 0; i < sizeof(routes) / sizeof(routes[0]); i++){
        http_server_register_uri( s_server, &routes[i]);
    }

    ESP_LOGI(TAG, "web app ready");
    return ESP_OK;
}













/* ----------------------------------------------------------
   PC -> Arduino
   POST /api/arduino/command
   ---------------------------------------------------------- */
// static esp_err_t handle_arduino_command(httpd_req_t *req){
//     char command[128];
//     int received = httpd_req_recv( req, command, sizeof(command) - 1);
//     if (received <= 0) {
//         httpd_resp_send_err( req, HTTPD_400_BAD_REQUEST, "No command received");
//         return ESP_FAIL;
//     }
//     command[received] = '\0';
//     esp_err_t err = uart_send_arduino_command(command);
//     if (err != ESP_OK) {
//         httpd_resp_send_err( req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send command");
//         return err;
//     }
//     httpd_resp_set_type( req, "text/plain");
//     httpd_resp_send( req, "OK", 2);
//     return ESP_OK;
// }







// #include "web_app.h"
// #include "http_server.h"
// #include "camera.h"
// #include "esp_log.h"

// #define TAG "web_app"

// extern const char index_html_start[] asm("_binary_index_html_start");
// extern const char index_html_end[]   asm("_binary_index_html_end");

// static httpd_handle_t s_server = NULL;

// static esp_err_t handle_index(httpd_req_t *req) {
//     httpd_resp_set_type(req, "text/html");
//     httpd_resp_send(req, index_html_start, index_html_end - index_html_start);
//     return ESP_OK;
// }

// esp_err_t web_app_start(void) {
//     esp_err_t err = http_server_start(&s_server);
//     if (err != ESP_OK) return err;

//     static const httpd_uri_t routes[] = {
//         { .uri = "/",                  .method = HTTP_GET, .handler = handle_index         },
//         { .uri = "/api/camera/stream", .method = HTTP_GET, .handler = handle_camera_stream },
//     };

//     for (int i = 0; i < sizeof(routes) / sizeof(routes[0]); i++) {
//         http_server_register_uri(s_server, &routes[i]);
//     }

//     ESP_LOGI(TAG, "web app ready");
//     return ESP_OK;
// }
