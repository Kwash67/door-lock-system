#ifndef DEBOUNCING_H
#define DEBOUNCING_H
#include "keypad.h"  // For NO_KEY definition

/**
 * @brief Debounces a character input (for example, a key event from a keypad).
 *
 * The debouncer requires a number of consecutive samples (threshold)
 * with the same input value before updating its stable state.
 * With a 10ms sampling period, a threshold of 5 gives roughly 50ms debounce.
 */
class KeyDebouncer {
public:
    /**
     * @brief Construct a debouncer.
     * @param threshold  Number of consecutive samples required to accept a change.
     * @param initialState The initial stable state (default is NO_KEY).
     */
    KeyDebouncer(uint8_t threshold = 5, char initialState = NO_KEY);

    /**
     * @brief Updates the debouncer with a fresh reading.
     * @param reading The new raw reading.
     * @return The debounced (stable) state.
     */
    char update(char reading);

    /**
     * @brief Returns the current debounced state.
     */
    char getState() const;

private:
    uint8_t threshold; // Number of consistent samples required.
    uint8_t counter;   // Counts successive samples that differ.
    char stableState;  // The accepted stable key value.
};

#endif // DEBOUNCING_H
