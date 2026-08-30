#include "http_server.h"
#include "esp_log.h"

#define TAG "http_server"

esp_err_t http_server_start(httpd_handle_t *server) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = 16;
    config.stack_size        = 8192;
    config.uri_match_fn      = httpd_uri_match_wildcard;
    esp_err_t err = httpd_start(server, &config);
    if (err == ESP_OK) ESP_LOGI(TAG, "started on port %d", config.server_port);
    return err;
}

esp_err_t http_server_stop(httpd_handle_t server) {
    return httpd_stop(server);
}

esp_err_t http_server_register_uri(httpd_handle_t server, const httpd_uri_t *uri) {
    return httpd_register_uri_handler(server, uri);
}
