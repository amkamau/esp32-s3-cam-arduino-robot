#include "uart_stream.h"

#include "driver/uart.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include <string.h>
#include <stdbool.h>


#define TAG "uart_stream"


#define UART_PORT     UART_NUM_1
#define UART_RX       21
#define UART_TX       14
#define UART_BAUD     115200

#define UART_RX_BUFFER_SIZE 1024
#define UART_TX_BUFFER_SIZE 1024

#define UART_TASK_STACK_SIZE 4096
#define UART_TASK_PRIORITY   5

#define MESSAGE_SIZE 128

static char robot_state[MESSAGE_SIZE];
static char arduino_command[MESSAGE_SIZE];

static SemaphoreHandle_t message_mutex;


/* ----------------------------------------------------------
   UART initialization
   ---------------------------------------------------------- */
esp_err_t uart_init(void){
    const uart_config_t uart_config = {
        .baud_rate = UART_BAUD,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    message_mutex = xSemaphoreCreateMutex();
    if (message_mutex == NULL) return ESP_ERR_NO_MEM;

    ESP_ERROR_CHECK(uart_driver_install(UART_PORT, UART_RX_BUFFER_SIZE, UART_TX_BUFFER_SIZE, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(UART_PORT, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT, UART_TX, UART_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    ESP_LOGI(TAG, "UART initialized");
    return ESP_OK;
}

/* ----------------------------------------------------------
   Low-level UART read
   ---------------------------------------------------------- */

int uart_read(uint8_t *data, size_t len, uint32_t timeout_ms){
    return uart_read_bytes( UART_PORT, data, len, pdMS_TO_TICKS(timeout_ms));
}


/* ----------------------------------------------------------
   Low-level UART write
   ---------------------------------------------------------- */
esp_err_t uart_write(const uint8_t *data, size_t len){    
    int written = uart_write_bytes(UART_PORT, data, len);    

    if (written < 0) { return ESP_FAIL; }

    if ((size_t)written != len) { return ESP_FAIL; }

    return ESP_OK;
}


/* ----------------------------------------------------------
   UART task
   ---------------------------------------------------------- */
static void uart_task(void *arg){
    uint8_t byte;

    char message[MESSAGE_SIZE];

    size_t index = 0;

    while (1) {
        /* ================================================
           Arduino -> ESP32 -> HTTP
           ================================================ */
        int len = uart_read( &byte, 1, 10);
        if (len > 0) {
            if (byte == '\n') {
                message[index] = '\0';
                if (xSemaphoreTake(message_mutex,portMAX_DELAY) == pdTRUE) {
                    strncpy(robot_state,message,MESSAGE_SIZE - 1);
                    robot_state[MESSAGE_SIZE - 1] = '\0';
                    xSemaphoreGive(message_mutex);
                } 
                ESP_LOGI(TAG,"Robot state: %s", message);
                index = 0;
            } else {
                if (index < MESSAGE_SIZE - 1) {
                    message[index++] = byte;
                } else {
                    ESP_LOGW(TAG,"Robot state message too long");
                    index = 0;
                }
            }
        }
        /* ================================================
           HTTP -> ESP32 -> Arduino
           ================================================ */
        char command[MESSAGE_SIZE];
        bool have_command = false;
        if (xSemaphoreTake( message_mutex, 0) == pdTRUE) {
            if (arduino_command[0] != '\0') {
                strncpy(command,arduino_command,MESSAGE_SIZE - 1);
                command[MESSAGE_SIZE - 1] = '\0';
                arduino_command[0] = '\0';
                have_command = true;
            }
            xSemaphoreGive(message_mutex);
        }
        if (have_command) {
            char message[MESSAGE_SIZE + 1];
            int len = snprintf(message, sizeof(message), "%s\n", command);
            uart_write((const uint8_t *)message, len);
            ESP_LOGI(TAG, "Arduino command: %s", command);
        }
    }
}


/* ----------------------------------------------------------
   Get latest robot state
   ---------------------------------------------------------- */
esp_err_t uart_get_robot_state(char *buffer,size_t buffer_size){
    if (buffer == NULL || buffer_size == 0) { return ESP_ERR_INVALID_ARG; }
    if (xSemaphoreTake(message_mutex, portMAX_DELAY) != pdTRUE) { return ESP_FAIL; }
    strncpy(buffer,robot_state,buffer_size - 1);
    buffer[buffer_size - 1] = '\0';
    xSemaphoreGive(message_mutex);
    return ESP_OK;
}

/* ----------------------------------------------------------
   HTTP provides command for Arduino
   ---------------------------------------------------------- */
esp_err_t uart_send_arduino_command(const char *command){

    if (command == NULL) { return ESP_ERR_INVALID_ARG; }
    if (strlen(command) >= MESSAGE_SIZE) { return ESP_ERR_INVALID_SIZE; }
    if (xSemaphoreTake(message_mutex,portMAX_DELAY) != pdTRUE) { return ESP_FAIL; }

    strcpy(arduino_command, command);
    xSemaphoreGive(message_mutex);

    return ESP_OK;
}

/* ----------------------------------------------------------
   Start UART task
   ---------------------------------------------------------- */
esp_err_t uart_start_task(void){
    BaseType_t result = xTaskCreate(uart_task, "uart_task", 4096, NULL, 5, NULL);
    if (result != pdPASS) {
        ESP_LOGE(TAG, "Failed to create UART task");
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "UART task started");
    return ESP_OK;
}







// esp_err_t uart_init(void){
//     const uart_config_t uart_config = {
//         .baud_rate = UART_BAUD,
//         .data_bits = UART_DATA_8_BITS,
//         .parity    = UART_PARITY_DISABLE,
//         .stop_bits = UART_STOP_BITS_1,
//         .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
//         .source_clk = UART_SCLK_DEFAULT,
//     };

//     esp_err_t err;

//     err = uart_driver_install(UART_PORT, UART_RX_BUFFER_SIZE, UART_TX_BUFFER_SIZE, 0,NULL, 0);
//     if (err != ESP_OK) { 
//         return err; 
//     }

//     err = uart_param_config(UART_PORT,&uart_config);
//     if (err != ESP_OK) { 
//         return err; 
//     }

//     err = uart_set_pin(UART_PORT, UART_TX, UART_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
//     if (err != ESP_OK) { 
//         return err; 
//     }

//     ESP_LOGI(TAG, "UART initialized");

//     return ESP_OK;
// }