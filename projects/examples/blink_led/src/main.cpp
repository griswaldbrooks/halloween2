#include <iostream>
#include <thread>
#include <chrono>
#include <cstdint>
#include "blink_controller.h"

/**
 * @brief Console output pin with colored terminal display
 *
 * Implements the same interface as mock_pin but outputs visual feedback
 * to the terminal with ANSI color codes:
 * - RED when LED is OFF
 * - GREEN when LED is ON
 *
 * Demonstrates the flexibility of blink_controller's dependency injection.
 */
struct console_led_pin {
public:
    /**
     * @brief Set LED state and display to console
     *
     * @param state true for ON (green), false for OFF (red)
     */
    void set(bool state) {
        state_ = state;

        // Get current timestamp
        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - start_time_);

        // ANSI color codes
        char const* green = "\033[32m";  // Green text
        char const* red = "\033[31m";    // Red text
        char const* reset = "\033[0m";   // Reset to default

        // Print colored status
        std::cout << "[" << duration.count() << "ms] LED: ";

        if (state_) {
            std::cout << green << "███ ON ███" << reset << std::endl;
        } else {
            std::cout << red << "▓▓▓ OFF ▓▓▓" << reset << std::endl;
        }
    }

    /**
     * @brief Get current LED state
     *
     * @return true LED is ON
     * @return false LED is OFF
     */
    bool get_state() const {
        return state_;
    }

    /**
     * @brief Reset the start time for timestamp display
     */
    void reset_time() {
        start_time_ = std::chrono::steady_clock::now();
    }

private:
    bool state_ = false;
    std::chrono::steady_clock::time_point start_time_ =
        std::chrono::steady_clock::now();
};

/**
 * @brief Simple timer that returns real-world milliseconds
 *
 * Provides the same interface as mock_timer but uses actual system time.
 */
struct real_time_timer {
public:
    real_time_timer() : start_time_(std::chrono::steady_clock::now()) {}

    /**
     * @brief Get milliseconds elapsed since timer creation
     *
     * @return uint32_t Elapsed time in milliseconds
     */
    uint32_t millis() const {
        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - start_time_);
        return static_cast<uint32_t>(duration.count());
    }

    /**
     * @brief Reset timer to zero
     */
    void reset() {
        start_time_ = std::chrono::steady_clock::now();
    }

private:
    std::chrono::steady_clock::time_point start_time_;
};

int main() {
    std::cout << "\n=== blink_controller Demo ===" << std::endl;
    std::cout << "Demonstrating dependency injection with console_led_pin\n" << std::endl;

    // Create console output pin
    console_led_pin console_pin;

    // Create timer
    real_time_timer timer;

    // Create blink controller
    // ON for 1000ms, OFF for 500ms (asymmetric blink)
    constexpr uint32_t ON_DURATION_MS = 1000;
    constexpr uint32_t OFF_DURATION_MS = 500;
    blink_controller<console_led_pin> controller(console_pin, ON_DURATION_MS, OFF_DURATION_MS);

    std::cout << "Configuration:" << std::endl;
    std::cout << "  ON duration:  " << ON_DURATION_MS << "ms" << std::endl;
    std::cout << "  OFF duration: " << OFF_DURATION_MS << "ms" << std::endl;
    std::cout << "  Total cycle:  " << (ON_DURATION_MS + OFF_DURATION_MS) << "ms" << std::endl;
    std::cout << "\nRunning for 10 seconds...\n" << std::endl;

    // Reset timers for synchronized display
    timer.reset();
    console_pin.reset_time();

    // Run simulation for 10 seconds
    constexpr uint32_t SIMULATION_DURATION_MS = 10000;
    constexpr uint32_t UPDATE_INTERVAL_MS = 50;  // Update every 50ms

    while (timer.millis() < SIMULATION_DURATION_MS) {
        uint32_t const current_time = timer.millis();

        // Update controller state
        controller.update(current_time);

        // Sleep to avoid busy-waiting
        std::this_thread::sleep_for(std::chrono::milliseconds(UPDATE_INTERVAL_MS));
    }

    std::cout << "\n=== Demo Complete ===" << std::endl;
    std::cout << "Notice how the controller manages timing and state transitions" << std::endl;
    std::cout << "while console_led_pin handles the output presentation." << std::endl;
    std::cout << "\nThis demonstrates the power of dependency injection:" << std::endl;
    std::cout << "  - Same blink_controller logic works with different output types" << std::endl;
    std::cout << "  - mock_pin for testing, console_led_pin for demo, hardware pins for production" << std::endl;
    std::cout << "  - Zero runtime overhead (template-based static polymorphism)" << std::endl;
    std::cout << "  - 100% testable business logic\n" << std::endl;

    return 0;
}
