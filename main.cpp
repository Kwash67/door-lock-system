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
FlashIAP flash;

const char admin_password[9] = "12345678";
char user_password[9] = "00000000";
char entered_password[9] = {'\0'}; 
char showed_password[9] = {'\0'};
int position = 0;
bool is_admin_mode = false;

void read_password_from_flash() 
{
    flash.init();  // Initialise Flash

    memset(user_password, 0, sizeof(user_password));  // Ensure "user_password" is empty

    // Read 4 bytes password from Flash
    flash.read(user_password, FLASH_STORAGE_ADDR, 8);

    user_password[8] = '\0';  // Ensure String is finished

    flash.deinit();  // Close Flash
}


void save_password_to_flash(const char *new_password) 
{
    flash.init();  // Initialise Flash

    uint32_t sector_size = flash.get_sector_size(FLASH_STORAGE_ADDR);

    //Create a temporary buffer and fill it with 0xFF
    char temp_buffer[sector_size];
    memset(temp_buffer, 0xFF, sector_size);  // Fill with 0xFF first

    // copy new password
    strncpy(temp_buffer, new_password, 8);  // Make sure you only save 4 characters.

    // remove Flash sector
    flash.erase(FLASH_STORAGE_ADDR, sector_size);

    //write new password
    flash.program(temp_buffer, FLASH_STORAGE_ADDR, 8);  //only write 4 bytes

    flash.deinit();  //close Flash
}



void blink_led(int times, char led_type, std::chrono::milliseconds(interval)) 
{
    if(led_type == 'g'){
        for (int i = 0; i < times; i++) 
        {
            led = 1;
            ThisThread::sleep_for(interval);
            led = 0;
            ThisThread::sleep_for(interval);
        }
    }
    else{ // if led_type == 'r'
        for (int i = 0; i < times; i++) 
        {
            led2 = 1;
            ThisThread::sleep_for(interval);
            led2 = 0;
            ThisThread::sleep_for(interval);
        }
    }
}

void led_on_for(int led_type, std::chrono::milliseconds(duration)) 
{
    if(led_type == 'g'){
        led = 1;
        ThisThread::sleep_for(duration);
        led = 0;
    }
    else{ // if led_type == 'r'
        led2 = 1;
        ThisThread::sleep_for(duration);
        led2 = 0;
    }
}

int main() 
{
    slcd.Home();  
    slcd.clear();
    read_password_from_flash();

    while (true) 
    {
        char key = keypad.ReadKey();
        if (key != NO_KEY) 
        {
            if (position < 8) 
            {
                memset(showed_password, ' ', 8);
                entered_password[8] = '\0';
                showed_password[8] = '\0';
                entered_password[position] = key;
                for(int i = 0; i <= position; i++)
                {
                    showed_password[7 - position + i] = entered_password[i];
                }

                position++;

                slcd.clear();
                slcd.Home();
                slcd.printf("%s", &showed_password[4]);

                if (key == '#') 
                {
                    slcd.clear();
                    position = 0;
                    memset(entered_password, 0, sizeof(entered_password));
                }

            }
            else if (position == 8 && key == '#')
            {
                slcd.clear();
                position = 0;
                memset(entered_password, 0, sizeof(entered_password));
            }

            else if (key == '*') //Confirm
            {
                entered_password[8] = '\0';
                if (position == 8 && strcmp(entered_password, admin_password) == 0) 
                {
                    slcd.clear();
                    slcd.printf("AD");
                    is_admin_mode = true;
                    position = 0;
                    memset(entered_password, 0, sizeof(entered_password));
                    ThisThread::sleep_for(1000ms);
                    slcd.clear();
                } 
                
                else if (is_admin_mode)//Manager Mode: Store the new password
                {
                    save_password_to_flash(entered_password);
                    read_password_from_flash();
                    slcd.clear();
                    slcd.printf("8888");  // Use 8888 to show saved
                    is_admin_mode = false;  //Exit manager mode
                } 

                else //Normal user mode: valide password
                {
                    if (strcmp(entered_password, user_password) == 0) 
                    {  
                        slcd.clear();
                        slcd.printf("YES");  // Show "TURE"
                        blink_led(5, 'g', std::chrono::milliseconds(300));  // LED flash 5 times
                    } 
                    else {  
                        slcd.clear();
                        slcd.printf("FA");  // Show "FALSE"
                        led_on_for('r', std::chrono::milliseconds(2000));  // LED hold 5 second
                    }
                }

                position = 0;
                memset(entered_password, 0, sizeof(entered_password));
                ThisThread::sleep_for(500ms);
                slcd.clear();
            }
        }
        ThisThread::sleep_for(80ms);
    }
}