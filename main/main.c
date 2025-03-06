#include "config.h"
#include "common.h" // Added include for message definitions
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_now.h"
#include <sys/time.h> // Required for settimeofday

#define TAG "PUMP_CONTROL"

// Initialize the function pointer for dependency injection
void (*gpio_set_level_ptr)(int pin, int level) = gpio_set_level;

static void send_esp_now_message(PumpCommandType cmd, const PumpCmdMessage* msg) {
    esp_err_t result = esp_now_send(MASTER_MAC_ADDRESS, (uint8_t*)msg, sizeof(PumpCmdMessage));
    if (result != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send message: %d", result);
    }
}

static void on_espnow_receive(const uint8_t *mac, const uint8_t *incomingData, int len) {
    if (len != sizeof(PumpCmdMessage)) return;

    PumpCmdMessage* msg = (PumpCmdMessage*)incomingData;
    
    switch(msg->command) {
        case CMD_SYNC_REQUEST: { // Handle sync command
            struct timeval tv;
            tv.tv_sec = msg->synced_time; 
            settimeofday(&tv, NULL);
            ESP_LOGI(TAG, "Synced system clock to %lu", (unsigned long)tv.tv_sec);
            break;
        }
        case CMD_STATUS_REQUEST: {
            // Prepare status reply using PumpStatusMessage
            PumpStatusMessage response = {CMD_STATUS_REQUEST};
            response.timestamp = time(NULL); 
            response.battery_soc = 42; // Dummy value - replace with actual reading
            for (int i=0; i<3; i++) {
                response.valve1 = gpio_get_level(VALVE_1_GPIO);
                response.valve2 = gpio_get_level(VALVE_2_GPIO);
                response.valve3 = gpio_get_level(VALVE_3_GPIO);
            }
            
            send_esp_now_message(CMD_STATUS_REPLY, &response); // Need to define CMD_STATUS_REPLY in enums
            break;
        }
        case CMD_START_PUMP: {
            ESP_LOGI(TAG, "Starting pump for %d seconds", msg->duration);
            gpio_set_level_ptr(PUMP_GPIO, 1);

            response.valve1 = msg->valve1; // Update valves based on message
            response.valve2 = msg->valve2;
            response.valve3 = msg->valve3;

            xTimerHandle timer = xTimerCreate("PumpStop", pdMS_TO_TICKS(msg->duration * 1000), pdFALSE, NULL, [](TimerHandle_t xTimer) {
                // Stop pump and valves
            });
            break;
        }
    }
}

void app_main() {
    // Initialization code remains similar but uses PumpCmdMessage structs
}
