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

    // Buffer to store last 4 entered characters (initialized with spaces)
    char inputBuffer[4] = {' ', ' ', ' ', ' '};
    int inputCount = 0;  // Tracks number of valid entries in buffer

    while (true) {
        led = !led;  // Toggle status LED every loop iteration
        
        // Read debounced key from keypad
        char key = keypad.ReadKey();

        if (key != NO_KEY) {  // Valid key detected
            led2 = 1;  // Illuminate keypress indicator
            
            if (key == '#') {  // Clear command
                md.clear();    // Clear all LCD segments
                md.Home();     // Reset cursor to first position
                memset(inputBuffer, ' ', sizeof(inputBuffer));  // Clear buffer
                inputCount = 0;  // Reset character counter
            } 
            else {  // Numeric key handling
                // Update circular buffer with new entry
                if (inputCount < 4) {
                    // Add to next available position
                    inputBuffer[inputCount++] = key;
                } else {
                    // Shift buffer left (oldest entry drops off)
                    for (int i = 0; i < 3; ++i) {
                        inputBuffer[i] = inputBuffer[i + 1];
                    }
                    inputBuffer[3] = key;  // Add new entry at end
                }
                
                // Update display with current buffer contents
                md.Home();  // Start at first display position
                for (int i = 0; i < 4; ++i) {
                    md.putc(inputBuffer[i]);  // Write each character
                }
            }
        } else {
            led2 = 0;  // Turn off keypress indicator
        }

        // Debounce delay and processor yield
        ThisThread::sleep_for(10ms);
    }
}