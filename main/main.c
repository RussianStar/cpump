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
    static xTimerHandle pump_timer = NULL; // Added timer handle
    if (len != sizeof(PumpCmdMessage)) return;

    PumpCmdMessage* msg = (PumpCmdMessage*)incomingData;
    
    switch(msg->command) {
        case CMD_SYNC_REQUEST: { // Handle sync command
            struct timeval old_tv, new_tv;
            gettimeofday(&old_tv, NULL);
            new_tv.tv_sec = msg->synced_time; 
            settimeofday(&new_tv, NULL);
            ESP_LOGI(TAG, "Synced system clock from %lu to %lu", (unsigned long)old_tv.tv_sec, (unsigned long)new_tv.tv_sec);
            break;
        }
        case CMD_STATUS_REQUEST: {
            // Prepare status reply using PumpStatusMessage
            PumpStatusMessage response = {CMD_STATUS_REPLY};
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

            if (pump_timer) {
                xTimerStop(pump_timer, 0);
                vTimerDelete(pump_timer);
            }
            pump_timer = xTimerCreate("PumpStop", 
                                    pdMS_TO_TICKS(msg->duration * 1000), 
                                    pdFALSE, 
                                    NULL, 
                                    [](TimerHandle_t xTimer) {
                                        // Close all valves when stopping
                                        gpio_set_level_ptr(VALVE_1_GPIO, 0);
                                        gpio_set_level_ptr(VALVE_2_GPIO, 0);
                                        gpio_set_level_ptr(VALVE_3_GPIO, 0);

                                        gpio_set_level_ptr(PUMP_GPIO, 0);
                                        
                                        // Send completion status
                                        PumpStatusMessage response;
                                        response.command = CMD_STATUS_REPLY;
                                        response.timestamp = time(NULL);
                                        response.battery_soc = 42; // Dummy value until real sensor is connected
                                        for (int i=0; i<3; i++) {
                                            response.valve1 = gpio_get_level(VALVE_1_GPIO);
                                            response.valve2 = gpio_get_level(VALVE_2_GPIO);
                                            response.valve3 = gpio_get_level(VALVE_3_GPIO);
                                        }
                                        
                                        send_esp_now_message(CMD_STATUS_REPLY, &response);
                                        
                                        ESP_LOGI(TAG, "Pump and valves stopped automatically");
                                        pump_timer = NULL;
                                    });
            if (pump_timer && xTimerStart(pump_timer, 0) != pdPASS) {
                ESP_LOGE(TAG, "Failed to start pump timer");
            }
            break;
        }

            case CMD_STOP_PUMP: { // New stop command handling
                if (pump_timer) {
                    xTimerStop(pump_timer, 0);
                    vTimerDelete(pump_timer);
                    pump_timer = NULL;
                }
                gpio_set_level_ptr(PUMP_GPIO, 0);

                // Close all valves when stopping
                gpio_set_level_ptr(VALVE_1_GPIO, 0);
                gpio_set_level_ptr(VALVE_2_GPIO, 0);
                gpio_set_level_ptr(VALVE_3_GPIO, 0);
                ESP_LOGI(TAG, "Pump and valves stopped manually");
                
                // Send status after stopping
                PumpStatusMessage response;
                response.command = CMD_STATUS_REPLY;
                response.timestamp = time(NULL);
                response.battery_soc = 42; // Dummy value until real sensor is connected
                for (int i=0; i<3; i++) {
                    response.valve1 = gpio_get_level(VALVE_1_GPIO);
                    response.valve2 = gpio_get_level(VALVE_2_GPIO);
                    response.valve3 = gpio_get_level(VALVE_3_GPIO);
                }
                
                send_esp_now_message(CMD_STATUS_REPLY, &response);
                break;
            }
    }
}

void app_main() {
    // Initialize ESP-NOW
    esp_now_init();
    esp_now_register_recv_cb(on_espnow_receive);

    // Send initial status report
    PumpStatusMessage init_status = {CMD_STATUS_REPLY};
    init_status.battery_soc = 42; // Dummy value until real sensor is connected
    send_esp_now_message(CMD_STATUS_REPLY, &init_status);
}
