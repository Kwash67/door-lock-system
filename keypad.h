#pragma once

#include "mbed.h"

// Value to indicate no key pressed
#define NO_KEY 0

class Keypad {
public:
    /**
     * Constructor for the Keypad class.
     * The pin assignments are provided as parameters—update these 
     * according to the wiring on your MKL46Z256 board along with the datasheets:
     *   e.g. row pins could be PTA14, PTA15, PTA16, PTA17 and
     *        column pins could be PTA5, PTA6, PTA7 (or update as needed).
     */
    Keypad(PinName PTA12 row0,
           PinName PTA4 row1,
           PinName PTA5 row2,
           PinName PTC8 row3,
           PinName PTD3 col0,
           PinName PTA2 col1,
           PinName PTA1 col2);

    /**
     * ReadKey() checks the latest key scan.
     * It returns a key value ONLY at the moment that
     * the key has changed since the last request.
     * Otherwise, it returns NO_KEY.
     */
    char ReadKey(void);

protected:
private:
    /**
     * KeyScanner() is launched as a background thread.
     * It sequentially scans each of the three keypad columns by
     * setting it low and reading the four rows.
     *
     * Mapping:
     *   Column 0: { '1', '4', '7', '#' }
     *   Column 1: { '2', '5', '8', '0' }
     *   Column 2: { '3', '6', '9', '*' }
     */
    void KeyScanner(void);

    // Thread that continuously scans the keys
    Thread keyscan;

    // The row input pins (digital reads)
    DigitalIn _row0;
    DigitalIn _row1;
    DigitalIn _row2;
    DigitalIn _row3;

    // The column output pins (digital writes)
    DigitalOut _col0;
    DigitalOut _col1;
    DigitalOut _col2;

    // Key mapping for a 3-column x 4-row keypad.
    char mapping[3][4] = {
        {'1', '4', '7', '#'},
        {'2', '5', '8', '0'},
        {'3', '6', '9', '*'}
    };

    // Shared variables between the KeyScanner thread and ReadKey.
    // 'key' holds the instantaneous key value,
    // 'key_p' tracks the previous key value reported.
    char key, key_p;
};

