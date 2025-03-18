/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#undef __ARM_FP // removes the error on the #include "mbed.h" line
#include "mbed.h"
#include "keypad.h"
#include "SLCD/SLCD.h"

// Blinking rate in milliseconds
#define BLINKING_RATE     500ms


int main() {
    // Initialize LCD display
    SLCD md;
    
    // Status LEDs (LED1 blinks continuously, LED2 shows keypress activity)
    DigitalOut led(LED1);
    DigitalOut led2(LED2);
    
    // Initialize keypad with specific pin assignments
    // Pins: Row0, Row1, Row2, Row3, Col0, Col1, Col2
    Keypad keypad(PTC8, PTA5, PTA4, PTA12, PTD3, PTA2, PTA1);

    // Display buffer: Stores last 4 entered characters in left-to-right order
    // Initialized with spaces to represent empty segments
    char inputBuffer[4] = {' ', ' ', ' ', ' '};
    int inputCount = 0;  // Tracks number of valid entries (0-4)

    // Initialize display: Clear all segments and set to known state
    md.clear();     // Turn off all LCD segments
    md.Home();      // Reset write position to first character
    md.printf("    ");  // Explicitly set empty display

    while (true) {
        led = !led;  // Toggle heartbeat LED (system alive indicator)
        
        // Read debounced key from keypad (non-blocking)
        char key = keypad.ReadKey();

        if (key != NO_KEY) {  // Valid key detected
            led2 = 1;  // Illuminate keypress indicator
            
            if (key == '#') {  // Clear command: Full display reset
                md.clear();    // SLCD native clear function (all segments off)
                md.Home();     // Reset cursor to position 0
                memset(inputBuffer, ' ', sizeof(inputBuffer));  // Clear buffer
                inputCount = 0;  // Reset character counter
                md.printf("    ");  // Force blank display (prevent ghost segments)
            } 
            else {  // Numeric input handling
                /* Buffer management:
                - When buffer full (4 entries), shift left to discard oldest
                - New entries always added to rightmost position */
                if (inputCount >= 4) {
                    // Shift buffer left (discard oldest entry at index 0)
                    for (int i = 0; i < 3; ++i) {
                        inputBuffer[i] = inputBuffer[i + 1];
                    }
                } else {
                    inputCount++;  // Track new entries until buffer full
                }
                
                // Add new entry to rightmost position
                inputBuffer[inputCount >= 4 ? 3 : inputCount - 1] = key;

                /* Display update:
                - Always start from home position (left alignment)
                - Use putc() for precise character control
                - Write spaces for empty buffer positions */
                md.Home();
                for (int i = 0; i < 4; ++i) {
                    inputBuffer[i] != ' ' ? md.putc(inputBuffer[i]) : md.putc(' ');
                }
            }
        } else {
            led2 = 0;  // Turn off keypress indicator
        }

        // 10ms delay: Keypad debouncing + processor yield
        ThisThread::sleep_for(10ms);
    }
}