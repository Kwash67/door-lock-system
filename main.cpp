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
char auth;
int tries = 0;
int max_tries = 2;

/**
* @brief Opens the door
*/
void openDoor(){
    slcd.printf("Open");
    for (int i = 0; i < 3; i++){
        slcd.printf("Open");
        ThisThread::sleep_for(200ms);
        slcd.clear();
        ThisThread::sleep_for(200ms);
    }
}

int main() 
{
    slcd.Home();  
    slcd.clear();
    InputModule inputModule(keypad, slcd);
    UserManagement userManager(keypad, slcd);

    while (true) 
    {
        if(tries <= max_tries) {
            
            // Collect user input
            while( !inputModule.hasPassword() ) {
                inputModule.processInput();
            }

            // Process the input once received
            if(inputModule.hasPassword()) {
                const char* password = inputModule.getEnteredPassword();
                auth = userManager.authenticate(password); // Check the auth level
            }

            if(auth == 'u'){
                // Normal User, Open Door
                openDoor();
                inputModule.reset();
            }
            else if(auth == 'a'){
                // Admin menu
                userManager.launch_admin();
                inputModule.reset();
            }
            else if(auth == 'x'){
                slcd.printf("FAIL");
                ThisThread::sleep_for(2s);
                tries++;
                inputModule.reset();
            }

        }
        else {
            // LOCKDOWN !!!!
            slcd.printf("WAIT");
            ThisThread::sleep_for(5s);
            tries = 0;
            inputModule.reset();
        }

        ThisThread::sleep_for(20ms);
    }
}