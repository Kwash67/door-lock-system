/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

 // #include "display.h"
 //int main()
 //display()

#include "mbed.h"
#include "keypad.h"
#include "SLCD.h"

// Blinking rate in milliseconds
#define BLINKING_RATE     500ms

SLCD md;

int s = 0;
int m = 0;
    // Initialise the digital pin LED1 as an output 
//#ifdef LED1
// Create an LED output (LED1) and a Keypad instance.
// These pin assignments match those in your keypad module.
// Update the pin names to match your hardware if necessary.
DigitalOut led(LED1);
Keypad Keypad(PTA12, PTA4, PTA5, PTC8, PTD3, PTA2, PTA1);

/*(PinName PTA12 row0,
           PinName PTA4 row1,
           PinName PTA5 row2,
           PinName PTC8 row3,
           PinName PTD3 col0,
           PinName PTA2 col1,
           PinName PTA1 col2);*/


int main() {
    while (true) {
        led = !led;
        md.printf("%02d%02d", m, s);
        md.Colon(1);
        ThisThread::sleep_for(BLINKING_RATE);
        md.Colon(0);
        ThisThread::sleep_for(BLINKING_RATE);
        s++;
        if (s>=60) {
            s = 0;
            m++;
        }

        // Read the debounced key from the keypad.
        char key = Keypad.ReadKey();
        
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

