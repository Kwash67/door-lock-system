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

#define FLASH_STORAGE_ADDR 0x0003FC00 //Storage address

Keypad keypad(PTC8, PTA5, PTA4, PTA12, PTD3, PTA2, PTA1);

SLCD slcd;
DigitalOut led(LED1);
DigitalOut led2(LED2);
char entered_password[9] = {'\0'}; 
int tries = 0;
int max_tries = 4;

// User Struct
typedef struct {
    char password[9];
    char role;
} User;

// List of users
#define MAX_USERS 10
User users[MAX_USERS] = {
    {"12345678", 'a'},  // Admin 
    {"00000000", 'u'},  // User 
};

char Compare(char* entered_password) {
    int user_count = sizeof(users) / sizeof(users[0]); // Current number of users in the array

    for (int i = 0; i < user_count; i++) {
        if (strcmp(entered_password, users[i].password) == 0) {
            return users[i].role;
        }
    }
    return 'x';  // No matching password found
}

void takePassword(void){
    //  Read the debounced key from the keypad.
    char key = keypad.ReadKey();
    slcd.Home();  
    slcd.clear();
    bool exit = false;
    while(!exit){
        if(key != NO_KEY){
            
        }
    }
}

int main() 
{
    while (true) 
    {
        if(tries <= max_tries) {
            slcd.printf("Entr");
            
            //  Accept input. Input module. Exit after Enter (#) is pressed.

            //  String compare logic. Returns 'a' for admin, 'u' for user, 'x' for wrong password
            char auth = Compare(entered_password);

            if(auth == 'u'){
                slcd.printf("Open");
                // turn on green LED
            }
            else if(auth == 'a'){
                slcd.printf("Admin");
                // Admin menu
            }
            else {
                slcd.printf("FAIL");
                tries++;
            }
        }
        else {
            slcd.printf("WAIT");
            ThisThread::sleep_for(10s);
            tries = 0;
        }

        ThisThread::sleep_for(80ms);
    }
}