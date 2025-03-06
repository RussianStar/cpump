#include <unity.h>
#include "config.h"

// Mock GPIO functions using dependency injection pointer
bool gpio_set_level_called = false;
void mock_gpio_set_level(int pin, int level) {
    if (pin == PUMP_GPIO && level == 1) {
        gpio_set_level_called = true;
    }
}

void setUp(void) {
    // Reset mocks and set function pointers
    gpio_set_level_ptr = mock_gpio_set_level;
    gpio_set_level_called = false;
}

void test_start_pump_sets_gpio() {
    // Arrange: Set up mock expectations

    // Act: Simulate receiving a start command
    EspNowMessage msg = {CMD_START_PUMP, 0, 0.0, 5, {0,0,0}};
    on_espnow_receive(MASTER_MAC_ADDRESS, (uint8_t*)&msg, sizeof(msg));

    // Assert
    TEST_ASSERT_TRUE(gpio_set_level_called);
}

void test_stop_pump_resets_gpio() {
    gpio_set_level_called = false;
    
    // Act: Simulate stop command
    EspNowMessage stop_msg = {CMD_STOP_PUMP};
    on_espnow_receive(MASTER_MAC_ADDRESS, (uint8_t*)&stop_msg, sizeof(stop_msg));

    TEST_ASSERT_TRUE(gpio_set_level_called);
}

// More tests to be added for other cases
