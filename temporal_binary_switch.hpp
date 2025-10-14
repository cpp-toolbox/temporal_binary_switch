#ifndef TEMPORAL_BINARY_SWITCH_HPP
#define TEMPORAL_BINARY_SWITCH_HPP

/**
 * @class TemporalBinarySwitch
 * @brief A binary switch that tracks temporal transitions between on and off states.
 *
 * Note that throughout these docs ^ will refer to an on signal and v is an off signal.
 *
 * This class manages a boolean state and provides functionality to detect
 * transitions from false → true (just switched on) and true → false (just switched off).
 * It also provides temporal query methods that reset their respective transition flags
 * after being checked.
 *
 * ### Example 1: Basic Usage
 * @code
 * #include "temporal_binary_switch.hpp"
 * #include <iostream>
 *
 * int main() {
 *     TemporalBinarySwitch toggle;
 *
 *     toggle.set_true();
 *
 *     if (toggle.just_switched_on()) {
 *         std::cout << "Switch just turned on!\n";
 *     }
 *
 *     if (toggle.just_switched_on_temporal()) {
 *         std::cout << "Temporal on detected!\n";
 *     }
 *
 *     // Subsequent temporal check will return false until it switches again
 *     if (!toggle.just_switched_on_temporal()) {
 *         std::cout << "No new on transition.\n";
 *     }
 *
 *     toggle.set_false();
 *
 *     if (toggle.just_switched_off_temporal()) {
 *         std::cout << "Switch just turned off!\n";
 *     }
 *
 *     return 0;
 * }
 * @endcode
 *
 * ### Example 2: Loop-based Usage
 * This example demonstrates detecting “just happened” transitions in a simulated update loop.
 * @code
 * #include "TemporalBinarySwitch.hpp"
 * #include <iostream>
 *
 * int main() {
 *     TemporalBinarySwitch input;
 *     bool simulated_button_state[] = {false, false, true, true, false, false};
 *
 *     for (int frame = 0; frame < 6; ++frame) {
 *         bool current_state = simulated_button_state[frame];
 *         if (current_state)
 *             input.set_true();
 *         else
 *             input.set_false();
 *
 *         if (input.just_switched_on_temporal())
 *             std::cout << "Frame " << frame << ": Button just pressed\n";
 *
 *         if (input.just_switched_off_temporal())
 *             std::cout << "Frame " << frame << ": Button just released\n";
 *     }
 *
 *     return 0;
 * }
 * @endcode
 *
 * @note The temporal query methods (`just_switched_on_temporal` and `just_switched_off_temporal`)
 *       consume their respective flags when called. This makes them ideal for polling within
 *       update loops or event-driven systems where you only need to react once per transition.
 */
class TemporalBinarySwitch {
  private:
    bool state = false;               ///< Current state of the switch (true = on, false = off)
    bool m_just_switched_on = false;  ///< True iff signal history is ...v^
    bool m_just_switched_off = false; ///< True iff signal history is ...^v
  public:
    /**
     * @brief Default constructor. Initializes the switch to an off state.
     */
    TemporalBinarySwitch() {}

    /**
     * @breif set the switch state depending on the incoming value
     *
     * internally calls set_true and set_false
     */
    void set(const bool &value) {
        if (value) {
            set_true();
        } else {
            set_false();
        }
    }

    /**
     * @brief Sets the switch state to true (on).
     *
     * If the previous state was false, this marks the switch as having just switched on.
     * If it was already true, no transition flags are modified.
     */
    void set_true() {
        if (!state) { // ...v^
            m_just_switched_on = true;
            m_just_switched_off = false;
        } else { // ...^^
            m_just_switched_on = false;
        }
        state = true;
    }

    /**
     * @brief Sets the switch state to false (off).
     *
     * If the previous state was true, this marks the switch as having just switched off.
     * If it was already false, no transition flags are modified.
     */
    void set_false() {
        if (state) { // ...^v
            m_just_switched_off = true;
            m_just_switched_on = false;
        } else { // ...vv
            m_just_switched_off = false;
        }
        state = false;
    }

    /**
     * @brief Checks if the switch has just switched on (non-temporal).
     *
     * This function does not modify internal state.
     * @return True if the switch has just transitioned to on; otherwise false.
     */
    bool just_switched_on() const { return m_just_switched_on; }

    /**
     * @brief Checks if the switch has just switched off (non-temporal).
     *
     * This function does not modify internal state.
     * @return True if the switch has just transitioned to off; otherwise false.
     */
    bool just_switched_off() const { return m_just_switched_off; }

    // NOTE: I don't think the below functions have much use...

    /**
     * @brief Checks if the switch has just switched on (temporal).
     *
     * This function returns true once per transition from off to on.
     * If called in succession, only the first call will return true
     *
     * @return True if the switch has just transitioned to on since the last check; otherwise false.
     */
    bool just_switched_on_temporal() {
        if (m_just_switched_on) {
            m_just_switched_on = false;
            return true;
        }
        return false;
    }

    /**
     * @brief Checks if the switch has just switched off (temporal).
     *
     * This function returns true once per transition from on to off.
     * If called in succession, only the first call will return true
     *
     * @return True if the switch has just transitioned to off since the last check; otherwise false.
     */
    bool just_switched_off_temporal() {
        if (m_just_switched_off) {
            m_just_switched_off = false;
            return true;
        }
        return false;
    }
};

#endif // TEMPORAL_BINARY_SWITCH_HPP
