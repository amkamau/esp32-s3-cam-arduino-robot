#pragma once

#include "esp_err.h"
#include "esp_http_server.h"

esp_err_t http_server_start(httpd_handle_t *server);
esp_err_t http_server_stop(httpd_handle_t server);
esp_err_t http_server_register_uri(httpd_handle_t server, const httpd_uri_t *uri);
