#include <iostream>
#include <thread>
#include <chrono>
#include <cstdint>
#include "blink_controller.h"

/**
 * @brief Console output pin with colored terminal display
 *
 * Implements the same interface as MockPin but outputs visual feedback
 * to the terminal with ANSI color codes:
 * - RED when LED is OFF
 * - GREEN when LED is ON
 *
 * Demonstrates the flexibility of BlinkController's dependency injection.
 */
class ConsoleLEDPin {
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
        const char* green = "\033[32m";  // Green text
        const char* red = "\033[31m";    // Red text
        const char* reset = "\033[0m";   // Reset to default

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
    bool getState() const {
        return state_;
    }

    /**
     * @brief Reset the start time for timestamp display
     */
    void resetTime() {
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
 * Provides the same interface as MockTimer but uses actual system time.
 */
class RealTimeTimer {
public:
    RealTimeTimer() : start_time_(std::chrono::steady_clock::now()) {}

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
    std::cout << "\n=== BlinkController Demo ===" << std::endl;
    std::cout << "Demonstrating dependency injection with ConsoleLEDPin\n" << std::endl;

    // Create console output pin
    ConsoleLEDPin console_pin;

    // Create timer
    RealTimeTimer timer;

    // Create blink controller
    // ON for 1000ms, OFF for 500ms (asymmetric blink)
    const uint32_t ON_DURATION_MS = 1000;
    const uint32_t OFF_DURATION_MS = 500;
    BlinkController<ConsoleLEDPin> controller(console_pin, ON_DURATION_MS, OFF_DURATION_MS);

    std::cout << "Configuration:" << std::endl;
    std::cout << "  ON duration:  " << ON_DURATION_MS << "ms" << std::endl;
    std::cout << "  OFF duration: " << OFF_DURATION_MS << "ms" << std::endl;
    std::cout << "  Total cycle:  " << (ON_DURATION_MS + OFF_DURATION_MS) << "ms" << std::endl;
    std::cout << "\nRunning for 10 seconds...\n" << std::endl;

    // Reset timers for synchronized display
    timer.reset();
    console_pin.resetTime();

    // Run simulation for 10 seconds
    const uint32_t SIMULATION_DURATION_MS = 10000;
    const uint32_t UPDATE_INTERVAL_MS = 50;  // Update every 50ms

    uint32_t last_update_time = 0;

    while (timer.millis() < SIMULATION_DURATION_MS) {
        uint32_t current_time = timer.millis();

        // Update controller state
        controller.update(current_time);

        // Sleep to avoid busy-waiting
        std::this_thread::sleep_for(std::chrono::milliseconds(UPDATE_INTERVAL_MS));
    }

    std::cout << "\n=== Demo Complete ===" << std::endl;
    std::cout << "Notice how the controller manages timing and state transitions" << std::endl;
    std::cout << "while ConsoleLEDPin handles the output presentation." << std::endl;
    std::cout << "\nThis demonstrates the power of dependency injection:" << std::endl;
    std::cout << "  - Same BlinkController logic works with different output types" << std::endl;
    std::cout << "  - MockPin for testing, ConsoleLEDPin for demo, hardware pins for production" << std::endl;
    std::cout << "  - Zero runtime overhead (template-based static polymorphism)" << std::endl;
    std::cout << "  - 100% testable business logic\n" << std::endl;

    return 0;
}
