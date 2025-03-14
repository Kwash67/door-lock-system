#include "debouncing.h"

KeyDebouncer::KeyDebouncer(uint8_t threshold, char initialState)
    : threshold(threshold), counter(0), stableState(initialState)
{
}

char KeyDebouncer::update(char reading) {
    if (reading == stableState) {
        // Same as the stable value: reset the counter.
        counter = 0;
    } else {
        // Different: increment the counter.
        counter++;
        // When the count meets or exceeds the threshold, accept the new value.
        if (counter >= threshold) {
            stableState = reading;
            counter = 0;
        }
    }
    return stableState;
}

char KeyDebouncer::getState() const {
    return stableState;
}
