#include <gtest/gtest.h>

#include "blink_controller.h"
#include "mock_hardware.h"

struct blink_controller_test : public ::testing::Test {
   protected:
    void SetUp() override {
        timer.reset();
        pin.reset();
    }

    mock_timer timer;
    mock_pin pin;
};

// Test constructor and initial state
TEST_F(blink_controller_test, constructor_initializes_correctly) {
    blink_controller<mock_pin> controller(pin, 1000, 500);

    EXPECT_EQ(controller.get_on_duration(), 1000);
    EXPECT_EQ(controller.get_off_duration(), 500);
    EXPECT_FALSE(controller.is_on());
    EXPECT_EQ(controller.get_last_toggle_time(), 0);
}

// Test initial state before any time passes
TEST_F(blink_controller_test, initial_state_is_off) {
    blink_controller<mock_pin> controller(pin, 1000, 500);

    controller.update(timer.millis());
    EXPECT_FALSE(pin.get_state());
    EXPECT_FALSE(controller.is_on());
}

// Test first transition from off to on
TEST_F(blink_controller_test, first_transition_off_to_on) {
    blink_controller<mock_pin> controller(pin, 1000, 500);

    // Initially off
    controller.update(timer.millis());
    EXPECT_FALSE(pin.get_state());

    // Still off before off_duration passes
    timer.advance(499);
    controller.update(timer.millis());
    EXPECT_FALSE(pin.get_state());

    // Should turn on after off_duration
    timer.advance(1);
    controller.update(timer.millis());
    EXPECT_TRUE(pin.get_state());
}

// Test second transition from on to off
TEST_F(blink_controller_test, second_transition_on_to_off) {
    blink_controller<mock_pin> controller(pin, 1000, 500);

    // Get to ON state
    timer.advance(500);
    controller.update(timer.millis());
    EXPECT_TRUE(pin.get_state());

    // Should stay on before on_duration passes
    timer.advance(999);
    controller.update(timer.millis());
    EXPECT_TRUE(pin.get_state());

    // Should turn off after on_duration
    timer.advance(1);
    controller.update(timer.millis());
    EXPECT_FALSE(pin.get_state());
}

// Test multiple complete cycles
TEST_F(blink_controller_test, multiple_cycles) {
    blink_controller<mock_pin> controller(pin, 1000, 500);

    // Cycle 1: Off -> On
    timer.advance(500);
    controller.update(timer.millis());
    EXPECT_TRUE(pin.get_state());

    // Cycle 1: On -> Off
    timer.advance(1000);
    controller.update(timer.millis());
    EXPECT_FALSE(pin.get_state());

    // Cycle 2: Off -> On
    timer.advance(500);
    controller.update(timer.millis());
    EXPECT_TRUE(pin.get_state());

    // Cycle 2: On -> Off
    timer.advance(1000);
    controller.update(timer.millis());
    EXPECT_FALSE(pin.get_state());

    // Cycle 3: Off -> On
    timer.advance(500);
    controller.update(timer.millis());
    EXPECT_TRUE(pin.get_state());
}

// Test reset functionality
TEST_F(blink_controller_test, reset_returns_to_initial_state) {
    blink_controller<mock_pin> controller(pin, 1000, 500);

    // Get to ON state
    timer.advance(500);
    controller.update(timer.millis());
    EXPECT_TRUE(controller.is_on());
    EXPECT_TRUE(pin.get_state());

    // Reset
    controller.reset();
    timer.reset();

    // Should be back to initial OFF state
    EXPECT_FALSE(controller.is_on());
    EXPECT_FALSE(pin.get_state());  // Pin should be set to LOW
    EXPECT_EQ(controller.get_last_toggle_time(), 0);

    // Should follow same pattern as initial startup
    controller.update(timer.millis());
    EXPECT_FALSE(pin.get_state());
    timer.advance(500);
    controller.update(timer.millis());
    EXPECT_TRUE(pin.get_state());
}

// Test time wraparound at UINT32_MAX
// Note: This is a complex edge case. In practice, wraparound occurs after ~49.7 days
// of continuous operation. The logic handles it correctly, but testing it requires
// careful setup of the controller state at specific times near UINT32_MAX.
// For this demonstration, we verify the wraparound calculation works with a simpler test.
TEST_F(blink_controller_test, handles_time_wraparound) {
    blink_controller<mock_pin> controller(pin, 100, 100);

    // Get controller to a state where last_toggle is near UINT32_MAX
    // Start at UINT32_MAX - 150
    controller.update(UINT32_MAX - 150);
    // LED toggled on (elapsed from 0 is huge)

    // Update at UINT32_MAX - 40 (110ms later, should toggle off)
    controller.update(UINT32_MAX - 40);
    EXPECT_FALSE(controller.is_on());  // 110ms elapsed > 100ms on_duration
    EXPECT_FALSE(pin.get_state());

    // Now wrap around: go from UINT32_MAX - 40 to 70
    // Elapsed: (UINT32_MAX - (UINT32_MAX-40)) + 70 + 1 = 40 + 70 + 1 = 111ms
    controller.update(70);
    EXPECT_TRUE(controller.is_on());  // 111ms > 100ms off_duration, should turn on
    EXPECT_TRUE(pin.get_state());
}

// Test state remains stable when called multiple times without time change
TEST_F(blink_controller_test, stable_state_when_time_unchanged) {
    blink_controller<mock_pin> controller(pin, 1000, 500);

    // Turn LED on
    timer.advance(500);
    controller.update(timer.millis());
    EXPECT_TRUE(pin.get_state());

    // Call update multiple times with same time
    controller.update(timer.millis());
    EXPECT_TRUE(pin.get_state());
    controller.update(timer.millis());
    EXPECT_TRUE(pin.get_state());
    controller.update(timer.millis());
    EXPECT_TRUE(pin.get_state());
}

// Test with different timing configurations
TEST_F(blink_controller_test, different_timing_configurations) {
    // Fast blink
    blink_controller<mock_pin> fast(pin, 100, 100);
    timer.advance(100);
    fast.update(timer.millis());
    EXPECT_TRUE(pin.get_state());
    timer.advance(100);
    fast.update(timer.millis());
    EXPECT_FALSE(pin.get_state());

    // Slow blink
    timer.reset();
    pin.reset();
    blink_controller<mock_pin> slow(pin, 5000, 5000);
    timer.advance(5000);
    slow.update(timer.millis());
    EXPECT_TRUE(pin.get_state());
    timer.advance(5000);
    slow.update(timer.millis());
    EXPECT_FALSE(pin.get_state());

    // Asymmetric blink (long on, short off)
    timer.reset();
    pin.reset();
    blink_controller<mock_pin> asymmetric(pin, 3000, 200);
    timer.advance(200);
    asymmetric.update(timer.millis());
    EXPECT_TRUE(pin.get_state());
    timer.advance(3000);
    asymmetric.update(timer.millis());
    EXPECT_FALSE(pin.get_state());
}

// Test edge case: zero duration (should toggle immediately)
TEST_F(blink_controller_test, zero_duration_toggles_immediately) {
    blink_controller<mock_pin> controller(pin, 0, 0);

    // First call should turn on immediately
    controller.update(timer.millis());
    EXPECT_TRUE(pin.get_state());

    // Second call should turn off immediately
    controller.update(timer.millis());
    EXPECT_FALSE(pin.get_state());

    // Third call should turn on again
    controller.update(timer.millis());
    EXPECT_TRUE(pin.get_state());
}

// Test that LED doesn't toggle prematurely
TEST_F(blink_controller_test, no_early_toggle) {
    blink_controller<mock_pin> controller(pin, 1000, 500);

    // Start in OFF state
    controller.update(timer.millis());
    EXPECT_FALSE(pin.get_state());

    // Advance just before toggle point
    for (uint32_t i = 1; i < 500; ++i) {
        timer.advance(1);
        controller.update(timer.millis());
        EXPECT_FALSE(pin.get_state());
    }

    // Now it should toggle
    timer.advance(1);
    controller.update(timer.millis());
    EXPECT_TRUE(pin.get_state());
}

// Test that output pin is updated on every update() call
TEST_F(blink_controller_test, output_pin_updated_every_call) {
    blink_controller<mock_pin> controller(pin, 1000, 500);
    uint32_t const initial_count = pin.get_toggle_count();

    // Each update() should call set() on the pin
    controller.update(timer.millis());
    EXPECT_GT(pin.get_toggle_count(), initial_count);

    uint32_t const count_after_first = pin.get_toggle_count();
    controller.update(timer.millis());
    EXPECT_GT(pin.get_toggle_count(), count_after_first);
}
