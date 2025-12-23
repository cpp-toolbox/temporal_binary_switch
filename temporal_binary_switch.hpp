#ifndef TEMPORAL_BINARY_SWITCH_HPP
#define TEMPORAL_BINARY_SWITCH_HPP

#include <string>
#include <vector>
#include <functional> // for std::reference_wrapper

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
 */
class TemporalBinarySwitch {

  public:
    enum class StateUpdateMethod {
        /**
         * @brief In this mode the state is updated whenever you call the set function, so you are manually changing the
         * state whenever you call the setters
         *
         * @code
         * #include "temporal_binary_switch.hpp"
         * #include <iostream>
         *
         * int main() {
         *     TemporalBinarySwitch toggle{TemporalBinarySwitch::StateUpdateMethod::manual};
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
         * Or in a loop without conditional logic [todo example here]
         *
         */
        manual,
        /**
         *
         * @brief In this mode the state is only updated whenever process is called, so the state doesn't change until
         * that occurs
         *
         * @details This is useful for situations when on each iteration of a loop the set function is called
         * conditionally. When this is the case if we were in manual mode, then internal state wouldn't update on ticks
         * when that conditional code is not run.
         *
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
         */
        process_synchronized
    };

    StateUpdateMethod state_update_method = TemporalBinarySwitch::StateUpdateMethod::process_synchronized;

    enum class State { sustained_off, just_switched_on, sustained_on, just_switched_off };

  private:
    State current_state;

    bool state_to_process = false; /// the state that will be used to update when processing (applies when we are in the
                                   /// process synchornized state update method)

    bool state = false; /// Current state of the switch (true = on, false = off)
  public:
    std::string to_string() const {
        if (just_switched_on()) {
            return "just switched on";
        }

        if (just_switched_off()) {
            return "just switched off";
        }

        if (sustained_on()) {
            return "sustained on";
        }

        if (sustained_off()) {
            return "sustained off";
        }

        return "unknown"; // fallback, should never happen
    }

    /**
     * @brief Default constructor. Initializes the switch to an off state.
     */
    TemporalBinarySwitch(
        const StateUpdateMethod &state_update_method = TemporalBinarySwitch::StateUpdateMethod::process_synchronized)
        : state_update_method(state_update_method) {}

    /**
     * @breif set the switch state depending on the incoming value
     *
     * internally calls set_true and set_false
     */
    void set(const bool &value) {
        switch (state_update_method) {
        case StateUpdateMethod::manual:
            update_state(value);
            break;
        case StateUpdateMethod::process_synchronized:
            state_to_process = value;
            break;
        }
    }

  private:
    void update_state(const bool &value) {
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
            current_state = State::just_switched_on;
        } else { // ...^^
            current_state = State::sustained_on;
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
            current_state = State::just_switched_off;
        } else { // ...vv
            current_state = State::sustained_off;
        }
        state = false;
    }

  public:
    const State &get_current_state() const { return current_state; }

    /**
     * @brief Returns true iff the signal is ...^
     */
    bool is_on() const { return state; }

    /**
     * @brief Returns true iff the signal is ...v
     */
    bool is_off() const { return !state; }

    /**
     * @brief Returns true iff the signal is ..v^
     */
    bool just_switched_on() const { return current_state == State::just_switched_on; }

    /**
     * @brief Returns true iff the signal is ..^v
     */
    bool just_switched_off() const { return current_state == State::just_switched_off; }

    /**
     * @brief Returns true iff the signal is ..^^
     */
    bool sustained_on() const { return current_state == State::sustained_on; }

    /**
     * @brief Returns true iff the signal is ..vv
     */
    bool sustained_off() const { return current_state == State::sustained_off; }

    /**
     * @brief Moves the current state along by repeating the previous state iff set has not be called since last time
     * this was called
     *
     * The purpose of this is for when a switch's set(...) call is inside a conditional block, and there's some external
     * loop that's iterating.
     *
     * If the conditional block doesn't run and the current state of the switch was just switched on, then the signal
     * will report that it is still in this state because the signal never progressed and modified its internal state.
     *
     * This is bad because with respect to the outer loop it will appear as though you'll have two ticks of the outer
     * loop where the signal will be in the just pressed state which might not be what you want.
     *
     * The remedy of to this will be to call the process function on each iteration of the outer loop which will keep
     * the signal up to date with respect to the outer loop.
     *
     * @note In specific cases you won't have to use the process function, such as when each iteration the set(...) is
     * not run conditionally.
     *
     */
    void process() {
        switch (state_update_method) {
        case StateUpdateMethod::process_synchronized:

            update_state(state_to_process);

            break;
        case StateUpdateMethod::manual:
            // do nothing.
            break;
        }
    }
};

#endif // TEMPORAL_BINARY_SWITCH_HPP
