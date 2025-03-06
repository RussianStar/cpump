#ifndef COMMON_H
#define COMMON_H

#include "esp_now.h"

// Message types shared between bridge and pump device
typedef enum {
    CMD_START_PUMP,
    CMD_STOP_PUMP,
    CMD_STATUS_REQUEST,
    CMD_SYNC_REQUEST,
    CMD_SYNC_RESPONSE,
    CMD_STATUS_REPLY, // Added missing status reply command
} PumpCommandType;
typedef struct {
    PumpCommandType command;
    int duration; // Used for start commands
    bool valve1, valve2, valve3; // New valve positions
    uint32_t synced_time;        // Time value from sync command (only valid for CMD_SYNC_REQUEST)
} PumpCmdMessage;

typedef struct {
    PumpCommandType original_cmd;
    uint32_t timestamp; // Current time in epoch seconds
    bool valve1, valve2, valve3; // Current valve positions
    uint8_t battery_soc; // 0-100%
    char status_text[50];
} PumpStatusMessage;

// Add new sync message struct (if needed)
typedef struct {
    uint32_t synced_time; // Time from master in epoch seconds
} SyncResponse;

// Update size check
_Static_assert((sizeof(PumpCmdMessage) + sizeof(PumpStatusMessage) + sizeof(SyncResponse)) <= 250, "Messages must not exceed 250 bytes for ESP-NOW compatibility");

#endif // COMMON_H
