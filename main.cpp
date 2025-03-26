/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#undef __ARM_FP // removes the error on the #include "mbed.h" line
#include "mbed.h"
#include "keypad.h"
#include "DigitalOut.h"
#include "FlashIAP.h"
#include "SLCD/SLCD.h"
#include "input_module.h"
#include "user_management.h"

Keypad keypad(PTC8, PTA5, PTA4, PTA12, PTD3, PTA2, PTA1);

SLCD slcd;
DigitalOut led(LED1);
DigitalOut led2(LED2);
char auth = 'h';
int tries = 0;
int max_tries = 4;

int main() 
{
    slcd.Home();  
    slcd.clear();
    InputModule inputModule(keypad, slcd);
    UserManagement userManager;

    slcd.printf("Entr");
    ThisThread::sleep_for(500ms);
    slcd.clear();

    while (true) 
    {
        if(tries <= max_tries) {
            
            while( !inputModule.hasPassword() ) {
                inputModule.processInput();
            }

            if(inputModule.hasPassword()) {
                const char* password = inputModule.getEnteredPassword();
                auth = userManager.authenticate(password);
                if(auth == 'u'){
                    // Normal User, Open Door
                    slcd.printf("Open");
                    led = 1;
                    ThisThread::sleep_for(2s);
                    led = 0;
                    auth = 'h'; // After operations, set auth to 'h'
                }
                else if(auth == 'a'){
                    // Admin menu
                    slcd.printf("Admin");
                    auth = 'h'; // After operations, set auth to 'h'
                }
                else if(auth == 'x'){
                    slcd.printf("FAIL");
                    tries++;
                    led2 = 1;
                    ThisThread::sleep_for(2s);
                    led2 = 0;
                    auth = 'h'; // After operations, set auth to 'h'
                }
            }

            else {   
                // Home
                slcd.printf("Entr");
            }
        }
        else {
            slcd.printf("WAIT");
            ThisThread::sleep_for(10s);
            tries = 0;
        }

        ThisThread::sleep_for(20ms);
    }
}