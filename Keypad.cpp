#include "keypad.h"
#include "ThisThread.h"

// The constructor initializes every DigitalIn and DigitalOut
// with the provided pin names and starts the key scanning thread.
Keypad::Keypad(PinName row0,
               PinName row1,
               PinName row2,
               PinName row3,
               PinName col0,
               PinName col1,
               PinName col2)
    : _row0(row0), 
      _row1(row1), 
      _row2(row2), 
      _row3(row3),
      _col0(col0), 
      _col1(col1), 
      _col2(col2)
{
    // Initialise keys to NO_KEY (i.e. no key pressed)
    key = NO_KEY;
    key_p = NO_KEY;

    // Start the key scanning thread with a 10ms period
    keyscan.start(callback(this, &Keypad::KeyScanner));
}

// This thread continually scans each column and checks each row input.
// Only one key press is reported per change.
void Keypad::KeyScanner(void) {
    char k;  // Local temporary for key detection

    while (true) {
        // Start with no key detected.
        k = NO_KEY;

        // --- Scan Column 0 ---
        _col0 = 0; _col1 = 1; _col2 = 1;
        if (_row0 == 0) k = mapping[0][0];
        if (_row1 == 0) k = mapping[0][1];
        if (_row2 == 0) k = mapping[0][2];
        if (_row3 == 0) k = mapping[0][3];

        // --- Scan Column 1 ---
        _col0 = 1; _col1 = 0; _col2 = 1;
        if (_row0 == 0) k = mapping[1][0];
        if (_row1 == 0) k = mapping[1][1];
        if (_row2 == 0) k = mapping[1][2];
        if (_row3 == 0) k = mapping[1][3];

        // --- Scan Column 2 ---
        _col0 = 1; _col1 = 1; _col2 = 0;
        if (_row0 == 0) k = mapping[2][0];
        if (_row1 == 0) k = mapping[2][1];
        if (_row2 == 0) k = mapping[2][2];
        if (_row3 == 0) k = mapping[2][3];

        // Update the shared key variable.
        key = k;

        // Sleep 10ms to debounce and to let the user see an instantaneous change.
        ThisThread::sleep_for(10ms);
    }
}

// ReadKey returns the newly pressed key (or NO_KEY if unchanged).
// It allows only one event per key press change.
char Keypad::ReadKey(void) {
    if (key != key_p) {
        key_p = key;
        return key;
    }
    return NO_KEY;
}

