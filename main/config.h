#ifndef CONFIG_H
#define CONFIG_H

// Configuration for Master Device MAC Address (must be set to your master's MAC)
#define MASTER_MAC_ADDRESS {0x1A, 0x2B, 0x3C, 0x4D, 0x5E, 0x6F}

// GPIO Pin Assignments
#define PUMP_GPIO 2
#define VALVE_1_GPIO 4
#define VALVE_2_GPIO 5
#define VALVE_3_GPIO 12

// Dependency injection for testing
extern void (*gpio_set_level_ptr)(int pin, int level);
// Command Types (to be used in ESP-NOW messages)
typedef enum {
    CMD_SYNC_REQUEST = 0,
    CMD_STATUS_REQUEST,
    CMD_START_PUMP,
    CMD_STOP_PUMP,
    CMD_SYNC_REPLY,
    CMD_STATUS_REPLY
} CommandType;

#endif // CONFIG_H
