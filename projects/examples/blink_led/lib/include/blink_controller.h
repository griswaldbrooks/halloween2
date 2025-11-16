#pragma once
#include <cstdint>

/**
 * @brief Platform-agnostic LED blink timing controller with dependency injection
 *
 * Template-based controller that accepts any output pin type via static polymorphism.
 * This enables:
 * - 100% testable business logic (using mock pins)
 * - Zero runtime overhead (no virtual dispatch)
 * - Hardware abstraction (works with any pin interface)
 * - Scalability (supports digital, PWM, multiple outputs, etc.)
 *
 * @tparam output_pin_t Type that implements set(bool) method
 *
 * Example Usage:
 *
 * // Hardware implementation
 * struct led_pin {
 *     void set(bool state) { digitalWrite(LED_PIN, state ? HIGH : LOW); }
 * };
 * led_pin pin;
 * blink_controller<led_pin> controller(pin, 1000, 500);
 * controller.update(millis());  // All logic handled internally
 *
 * // Test implementation
 * struct mock_pin {
 *     bool state = false;
 *     void set(bool s) { state = s; }
 * };
 * mock_pin mock;
 * blink_controller<mock_pin> controller(mock, 1000, 500);
 * controller.update(0);
 * ASSERT_FALSE(mock.state);
 */
template<typename output_pin_t>
struct blink_controller {
   public:
    /**
     * @brief Construct a new blink controller
     *
     * @param output Reference to output pin interface
     * @param on_duration_ms How long LED stays on (milliseconds)
     * @param off_duration_ms How long LED stays off (milliseconds)
     */
    blink_controller(output_pin_t& output, uint32_t on_duration_ms, uint32_t off_duration_ms)
        : output_(output),
          on_duration_ms_(on_duration_ms),
          off_duration_ms_(off_duration_ms),
          last_toggle_time_ms_(0),
          led_on_(false) {}

    /**
     * @brief Update LED state based on current time
     *
     * Call this in your main loop with the current time.
     * Handles state transitions AND output control automatically.
     * This method now contains ALL logic - no business logic in .ino!
     *
     * @param current_time_ms Current time in milliseconds
     */
    void update(uint32_t current_time_ms) {
        // Calculate time elapsed since last toggle
        uint32_t elapsed;
        if (current_time_ms >= last_toggle_time_ms_) {
            // Normal case: no wraparound
            elapsed = current_time_ms - last_toggle_time_ms_;
        } else {
            // Handle uint32_t wraparound (occurs after ~49.7 days)
            elapsed = (UINT32_MAX - last_toggle_time_ms_) + current_time_ms + 1;
        }

        // Determine if we should toggle
        uint32_t const target_duration = led_on_ ? on_duration_ms_ : off_duration_ms_;

        if (elapsed >= target_duration) {
            // Toggle the LED state
            led_on_ = !led_on_;
            last_toggle_time_ms_ = current_time_ms;
        }

        // Output control - ALL logic testable!
        output_.set(led_on_);
    }

    /**
     * @brief Reset controller to initial state
     *
     * LED will be off, timer will be reset to 0.
     * Also updates the output pin to OFF state.
     */
    void reset() {
        last_toggle_time_ms_ = 0;
        led_on_ = false;
        output_.set(false);
    }

    // Getters for testing and state inspection
    uint32_t get_on_duration() const { return on_duration_ms_; }
    uint32_t get_off_duration() const { return off_duration_ms_; }
    bool is_on() const { return led_on_; }
    uint32_t get_last_toggle_time() const { return last_toggle_time_ms_; }

   private:
    output_pin_t& output_;
    uint32_t on_duration_ms_;
    uint32_t off_duration_ms_;
    uint32_t last_toggle_time_ms_;
    bool led_on_;
};
