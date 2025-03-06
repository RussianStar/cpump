#include <unity.h>
#include "config.h"

// Mock GPIO functions using dependency injection pointer
bool gpio_set_level_called = false;
void mock_gpio_set_level(int pin, int level) {
    // Track all GPIO operations for valves and pump
    static bool valve1_set = false;
    static bool valve2_set = false;
    static bool valve3_set = false;

    if (pin == PUMP_GPIO && level == 1) {
        gpio_set_level_called = true;
    } else if (pin == VALVE_1_GPIO) {
        valve1_set = true;
    } else if (pin == VALVE_2_GPIO) {
        valve2_set = true;
    } else if (pin == VALVE_3_GPIO) {
        valve3_set = true;
    }
}

void test_valves_set_on_start() {
    // Arrange
    const int expected_valve1 = 1;
    const int expected_valve2 = 0;
    const int expected_valve3 = 1;

    PumpCmdMessage msg = {CMD_START_PUMP, 5};
    msg.valve1 = expected_valve1;
    msg.valve2 = expected_valve2;
    msg.valve3 = expected_valve3;

    // Reset mocks
    gpio_set_level_called = false;
    valve1_set = false;
    valve2_set = false;
    valve3_set = false;

    // Act: Simulate start command
    on_espnow_receive(MASTER_MAC_ADDRESS, (uint8_t*)&msg, sizeof(msg));

    // Assert pump started and valves set correctly
    TEST_ASSERT_TRUE(gpio_set_level_called);
    TEST_ASSERT_EQUAL(expected_valve1, valve1_set);
    TEST_ASSERT_EQUAL(expected_valve2, valve2_set);
    TEST_ASSERT_EQUAL(expected_valve3, valve3_set);
}

void test_valves_closed_on_stop_manual() {
    // Arrange: Simulate active pump
    PumpCmdMessage start_msg = {CMD_START_PUMP, 10};
    on_espnow_receive(MASTER_MAC_ADDRESS, (uint8_t*)&start_msg, sizeof(start_msg));

    // Reset mocks for stop operation
    gpio_set_level_called = false;
    valve1_set = false;
    valve2_set = false;
    valve3_set = false;

    // Act: Send manual stop command
    PumpCmdMessage stop_msg = {CMD_STOP_PUMP};
    on_espnow_receive(MASTER_MAC_ADDRESS, (uint8_t*)&stop_msg, sizeof(stop_msg));

    // Assert all valves closed (set to 0)
    TEST_ASSERT_TRUE(gpio_set_level_called); // Pump stopped
    TEST_ASSERT_EQUAL(0, valve1_set);
    TEST_ASSERT_EQUAL(0, valve2_set);
    TEST_ASSERT_EQUAL(0, valve3_set);
}

void test_valves_closed_on_auto_stop() {
    // Arrange: Start pump with 1-second duration
    PumpCmdMessage start_msg = {CMD_START_PUMP, 1};
    on_espnow_receive(MASTER_MAC_ADDRESS, (uint8_t*)&start_msg, sizeof(start_msg));

    // Wait for timer to expire (mock this behavior)
    vTaskDelay(2000 / portTICK_PERIOD_MS); // Wait longer than duration

    // Reset mocks after start
    gpio_set_level_called = false;
    valve1_set = false;
    valve2_set = false;
    valve3_set = false;

    // Act: Let timer trigger auto-stop (mocked by manual callback call)
    xTimerHandle dummy_timer = xTimerCreate("TestTimer", 0, pdFALSE, NULL, [](TimerHandle_t t) {
        on_espnow_receive(NULL, NULL, 0); // Not needed - just let the timer callback run
    });
    xTimerCallbackFunction(dummy_timer);

    // Assert valves closed after auto-stop
    TEST_ASSERT_TRUE(gpio_set_level_called);
    TEST_ASSERT_EQUAL(0, valve1_set);
    TEST_ASSERT_EQUAL(0, valve2_set);
    TEST_ASSERT_EQUAL(0, valve3_set);
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
