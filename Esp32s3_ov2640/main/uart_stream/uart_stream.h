#pragma once

#include "esp_err.h"
#include <stdint.h>
#include <stddef.h>

esp_err_t uart_init(void);
int uart_read(uint8_t *data,size_t len,uint32_t timeout_ms);
esp_err_t uart_write(const uint8_t *data,size_t len);
esp_err_t uart_get_robot_state(char *buffer,size_t buffer_size);
esp_err_t uart_send_arduino_command(const char *command);
esp_err_t uart_start_task(void);