#pragma once

#include "esp_err.h"
#include "esp_http_server.h"

esp_err_t camera_init(void);
esp_err_t handle_camera_stream(httpd_req_t *req);
