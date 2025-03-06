# Pump Control System

## Setup Instructions

### Prerequisites
- ESP-IDF Framework installed (https://docs.espressif.com/projects/esp-idf/en/latest/)
- ESP32 Development Board flashed with bootloader and partitions

### Configuration
1. Open `main/config.h` and set the MASTER_MAC_ADDRESS to your master device's MAC address:
```c
#define MASTER_MAC_ADDRESS {0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX}
```

2. Configure GPIO pins in `config.h` if needed (VALVE_1_GPIO etc.)

### Building & Flashing
```bash
idf.py build
idf.py flash
idf.py monitor
```

The system will automatically start on device reboot.

## Features
- Pump control via ESP-NOW commands (`CMD_START_PUMP`, `CMD_STOP_PUMP`)
- Automatic valve management during shutdowns
- Immediate status report sent to master device upon boot
- On-demand status reports when requested


## Testing
Run unit tests with:
```bash
cd tests && make test
