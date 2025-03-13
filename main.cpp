/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include "keypad.h"
#include "SLCD.h"

// Blinking rate in milliseconds
#define BLINKING_RATE     500ms

SLCD md;

int main()
{ int s = 0;
  int m = 0;
    // Initialise the digital pin LED1 as an output 
//#ifdef LED1
DigitalOut led(LED1);
//#else
//    bool led;


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
    }
}
