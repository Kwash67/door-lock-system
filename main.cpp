/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include "keypad.h"

// Create an LED output (LED1) and a Keypad instance.
// These pin assignments match those in your keypad module.
// Update the pin names to match your hardware if necessary.
DigitalOut led(LED1);
Keypad keypad(PTA14, PTA15, PTA16, PTA17, PTA5, PTA6, PTA7);

int main() {
    while (true) {
        // Read the debounced key from the keypad.
        char key = keypad.ReadKey();
        
        // Turn on the LED if any key is pressed (i.e. key is not NO_KEY).
        if(key != NO_KEY) {
            led = 1;
        } else {
            led = 0;
        }
        
        // Short delay so the loop doesn't hog the processor.
        ThisThread::sleep_for(10ms);
    }
}

