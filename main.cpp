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

// Display buffer (stores last 4 digits, left to right)
    char inputBuffer[4] = {' ', ' ', ' ', ' '};
    int inputCount = 0;  // Number of stored digits

    // Initialize display with blank spaces
    md.clear();     // Turn off all LCD segments
    md.Home();      // Set write position to start
    md.printf("    ");  // Explicit blank display

    while (true) {
        led = !led;  // Toggle heartbeat LED
        
        char key = keypad.ReadKey();  // Get debounced keypress

        if (key != NO_KEY) {
            led2 = 1;  // Light keypress indicator
            
            if (key == '#') {  // FULL DISPLAY CLEAR
                /* Clear Procedure:
                1. Use SLCD's hardware clear function
                2. Reset software buffer
                3. Explicitly blank all positions */
                
                md.clear();     // 1. Turn off all segments
                md.Home();      // Reset cursor to start
                memset(inputBuffer, ' ', sizeof(inputBuffer)); // 2. Clear buffer
                inputCount = 0; // Reset digit counter
                
                // 3. Force blank all display positions
                for(int i = 0; i < 4; i++) {
                    md.putc(' ');  // Clear each digit position
                }
            } 
            else {  // HANDLE NUMBER INPUT
                /* Buffer Management:
                - Maintain last 4 entered digits
                - Shift old entries left when full */
                if(inputCount < 4) {
                    // Add to next available position
                    inputBuffer[inputCount++] = key;
                } else {
                    // Shift buffer left (discard oldest)
                    for(int i = 0; i < 3; i++) {
                        inputBuffer[i] = inputBuffer[i+1];
                    }
                    inputBuffer[3] = key;  // Add new to right
                }

                // Update display with current buffer
                md.Home();  // Start from leftmost position
                for(int i = 0; i < 4; i++) {
                    md.putc(inputBuffer[i]);  // Write each character
                }
            }
        } else {
            led2 = 0;  // Turn off keypress indicator
        }

        // 10ms delay for debounce and CPU yield
        ThisThread::sleep_for(10ms);
    }
}