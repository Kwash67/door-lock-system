/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#undef __ARM_FP // removes the error on the #include "mbed.h" line
#include "mbed.h"
#include "keypad.h"
#include "SLCD.h"

// Blinking rate in milliseconds
#define BLINKING_RATE     500ms


int main() {
    SLCD md;

    int s = 0;
    int m = 0;

    // System Status LED
    DigitalOut led(LED1);
    DigitalOut led2(LED2);

    //Instantiation of the keypad
    Keypad Keypad(PTC8, PTA5, PTA4, PTA12, PTD3, PTA2, PTA1);
    // Keypad Keypad(PTA12, PTA4, PTA5, PTC8, PTD3, PTA2, PTA1);

    while (true) {
        led = !led;
        // md.printf("%02d%02d", m, s);
        // md.Colon(1);
        // ThisThread::sleep_for(BLINKING_RATE);
        // md.Colon(0);
        // ThisThread::sleep_for(BLINKING_RATE);
        // s++;
        // if (s>=60) {
        //     s = 0;
        //     m++;
        // }

        // Read the debounced key from the keypad.
        char key = Keypad.ReadKey();
        
        // Turn on the LED if any key is pressed (i.e. key is not NO_KEY).
        if(key != NO_KEY) {
            led2 = 1;  //Access control probably needs to be implemented here 
            md.printf("%c",key);
        } else {
            led2 = 0;
        }
        
        // Short delay so the loop doesn't hog the processor.
        ThisThread::sleep_for(10ms);
    }
}

