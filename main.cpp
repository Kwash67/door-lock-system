/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

// Remove potential ARM_FP issues.
#undef __ARM_FP 
#include "mbed.h"
#include "keypad.h"
#include "DigitalOut.h"
#include "FlashIAP.h"
#include "SLCD/SLCD.h"
#include "debouncing.h"
#include <cstring>

//-----------------------------------------------------------------
// Flash storage layout:
//   - First 8 bytes: admin password (stored as 8-character string, only 4–8 digits used)
//   - Next 9 x 8 bytes: nine user passwords (each 8 characters)
// Total storage bytes = 8 + (9*8) = 80 bytes
//-----------------------------------------------------------------
#define FLASH_STORAGE_ADDR 0x0003FC00  // Recommended storage address
#define TOTAL_STORAGE_BYTES 80         // 8 (admin) + 9*8 (users)

// Global objects
Keypad keypad(PTC8, PTA5, PTA4, PTA12, PTD3, PTA2, PTA1);
SLCD slcd;
DigitalOut led(LED1);
DigitalOut led2(LED2);
FlashIAP flash;
KeyDebouncer debouncer(5); // 50ms debounce period

// Global password arrays (each string has space for 8 digits + null terminator)
char admin_password[9];             // Admin password (modifiable)
char user_passwords[9][9];            // Nine user passwords

// Buffers used for input (up to 8 digits) and for building a right-aligned display string.
char entered_password[9] = { '\0' };
char showed_password[9] = { '\0' };   // 8-char buffer used to right-align input.
int position = 0;                   // Current number of entered digits

//----------------------------------------------------------------------
// System State Machine for multi-user functionality
//----------------------------------------------------------------------
enum SystemState {
    STATE_NORMAL,          // Regular user login mode.
    STATE_ADMIN_MENU,      // Admin mode: select which password to update.
    STATE_SET_NEW_PASSWORD, // Admin enters new password for the selected slot.
    STATE_DELETE_OLD_PASSWORD // Delete the old password.
};

SystemState state = STATE_NORMAL;
int current_selection = -1;  // In admin menu: -1 for admin update; 0-8 for user password update.

//----------------------------------------------------------------------
// Flash functions: read and write all passwords.
//----------------------------------------------------------------------

/** 
 * @brief Reads admin and user passwords from flash memory.
 * Loads default values if flash area is unprogrammed (0xFF).
 */
void read_all_passwords_from_flash() {
    flash.init();
    char buffer[TOTAL_STORAGE_BYTES];
    flash.read(buffer, FLASH_STORAGE_ADDR, TOTAL_STORAGE_BYTES);
    flash.deinit();
    
    // Load admin password (first 8 bytes)
    if ((unsigned char)buffer[0] == 0xFF) {
        strcpy(admin_password, "12345678"); // Default admin password
    } else {
        memcpy(admin_password, buffer, 8);
        admin_password[8] = '\0';
    }
    
    // Load nine user passwords (each 8 bytes)
    for (int i = 0; i < 9; i++) {
        if ((unsigned char)buffer[8 + i * 8] == 0xFF) {
            strcpy(user_passwords[i], "00000000"); // Default user password
        } else {
            memcpy(user_passwords[i], buffer + 8 + i * 8, 8);
            user_passwords[i][8] = '\0';
        }
    }
}

/**
 * @brief Saves all passwords (admin and users) to flash memory.
 * Uses a temporary buffer to update the entire flash block.
 */
void save_all_passwords_to_flash() {
    flash.init();
    uint32_t sector_size = flash.get_sector_size(FLASH_STORAGE_ADDR);
    char temp_buffer[sector_size];
    memset(temp_buffer, 0xFF, sector_size);  // Pre-fill buffer with 0xFF
    
    // Copy admin password (8 bytes)
    strncpy(temp_buffer, admin_password, 8);
    
    // Copy nine user passwords (each 8 bytes)
    for (int i = 0; i < 9; i++) {
        strncpy(&temp_buffer[8 + i * 8], user_passwords[i], 8);
    }
    
    flash.erase(FLASH_STORAGE_ADDR, sector_size);
    flash.program(temp_buffer, FLASH_STORAGE_ADDR, TOTAL_STORAGE_BYTES);
    flash.deinit();
}

//----------------------------------------------------------------------
// LED helper functions for visual feedback
//----------------------------------------------------------------------

/**
 * @brief Blink a specified LED a given number of times.
 * @param times Number of blinks.
 * @param led_type 'g' for LED1 (green) or 'r' for LED2 (red).
 * @param interval Duration for each on/off period.
 */
void blink_led(int times, char led_type, std::chrono::milliseconds interval) {
    if (led_type == 'g') {
        for (int i = 0; i < times; i++) {
            led = 1;
            ThisThread::sleep_for(interval);
            led = 0;
            ThisThread::sleep_for(interval);
        }
    } else { // 'r' for red LED (LED2)
        for (int i = 0; i < times; i++) {
            led2 = 1;
            ThisThread::sleep_for(interval);
            led2 = 0;
            ThisThread::sleep_for(interval);
        }
    }
}

/**
 * @brief Turn on a specified LED for a given duration.
 * @param led_type 'g' for LED1 (green) or 'r' for LED2 (red).
 * @param duration Time duration to keep the LED on.
 */
void led_on_for(char led_type, std::chrono::milliseconds duration) {
    if (led_type == 'g') {
        led = 1;
        ThisThread::sleep_for(duration);
        led = 0;
    } else { // 'r' for red LED
        led2 = 1;
        ThisThread::sleep_for(duration);
        led2 = 0;
    }
}

//----------------------------------------------------------------------
// Helper function to update and display the masked (right-aligned) input.
// This function builds an 8-character buffer with spaces and copies the
// entered characters to the right side, then prints the last 4 characters.
//----------------------------------------------------------------------
void update_input_display() {
    // Fill the entire 8-character buffer with spaces.
    memset(showed_password, ' ', 8);
    showed_password[8] = '\0';
    
    // Copy the entered password into the buffer, right-aligned.
    // If position is the number of entered digits, then copy into indices (8 - position) to 7.
    for (int i = 0; i < position; i++) {
        showed_password[8 - position + i] = entered_password[i];
    }
    
    // Clear display and print last 4 characters (indices 4..7).
    slcd.clear();
    slcd.Home();
    slcd.printf("%.*s", 4, &showed_password[4]);
}

//----------------------------------------------------------------------
// Helper function to clear the input buffers and reset position.
 //----------------------------------------------------------------------
void clear_input() {
    position = 0;
    memset(entered_password, 0, sizeof(entered_password));
    memset(showed_password, 0, sizeof(showed_password));
}

//----------------------------------------------------------------------
// Helper function to display a 4-character message, padded with spaces if needed.
//----------------------------------------------------------------------

void display_message(const char* msg) {
    char buf[5] = "    ";  // 4 spaces, plus null terminator.
    // Copy up to 4 characters from msg into buf.
    strncpy(buf, msg, 4);
    buf[4] = '\0';
    slcd.clear();
    slcd.Home();
    slcd.printf("%s", buf);
}

//----------------------------------------------------------------------
// Main function: implements the multi-user state machine.
//----------------------------------------------------------------------
int main() {
    // Initialize the display and load passwords from flash.
    slcd.Home();
    slcd.clear();
    read_all_passwords_from_flash();
    
    // Show a welcome message for 1 second (ensure 4-character display).
    display_message("Hey");
    ThisThread::sleep_for(1000ms);
    slcd.clear();
    
    // Main loop: continuously poll the keypad.
    while (true) {
        char key = keypad.ReadKey();
        if (key != NO_KEY) {
            // Allow cancellation: '*' clears input and resets to normal mode.
            if (key == '*') {
                clear_input();
                state = STATE_NORMAL;
                slcd.clear();
                continue;
            }
            
            switch (state) {
                //----- STATE: NORMAL (Regular login mode) -----
                case STATE_NORMAL:
                    // Accept digits until '#' is pressed or max 8 digits reached.
                    if (key != '#' && position < 8) {
                        // Append key to entered password.
                        entered_password[position] = key;
                        position++;
                        // Update and display masked (right-aligned) input.
                        update_input_display();
                    }
                    else if (key == '#' && position > 0) {
                        entered_password[position] = '\0';
                        
                        // Check for minimum length (4 digits).
                        if (position < 4) {
                            display_message("INSF");  // "INSF" = insufficient
                            led_on_for('r', 1000ms);
                            clear_input();
                            continue;
                        }
                        
                        // Pad with zeros if fewer than 8 digits.
                        if (position < 8) {
                            for (int i = position; i < 8; i++) {
                                entered_password[i] = '0';
                            }
                            entered_password[8] = '\0';
                        }
                        
                        // Check if the entered password matches the admin password.
                        if (strcmp(entered_password, admin_password) == 0) {
                            display_message("Ad  ");  // "Ad" = admin mode
                            ThisThread::sleep_for(1000ms);
                            slcd.clear();
                            state = STATE_ADMIN_MENU;
                            clear_input();
                        }
                        else {
                            // Check if entered password matches any of the nine user passwords.
                            bool user_valid = false;
                            for (int i = 0; i < 9; i++) {
                                if (strcmp(entered_password, user_passwords[i]) == 0) {
                                    user_valid = true;
                                    break;
                                }
                            }
                            if (user_valid) {
                                display_message("yes ");  // "yes " = successful login
                                blink_led(5, 'g', 300ms);
                            } else {
                                display_message("FA  ");  // "FA  " = failure
                                led_on_for('r', 2000ms);
                            }
                            clear_input();
                        }
                    }
                break; // end STATE_NORMAL
                
                //----- STATE: ADMIN_MENU (Admin selects which password to update) -----
                case STATE_ADMIN_MENU:
                    // In admin mode:
                    //   Key '0' selects updating the admin password.
                    //   Keys '1' through '9' select the corresponding user password.
                    if (key == '0') {
                        current_selection = -1;  // -1 indicates admin password update.
                        display_message("ChAd");  // "CDAd" = Change or Delete admin (short for admin)
                        clear_input();
                        state = STATE_SET_NEW_PASSWORD;
                    }
                    else if (key >= '1' && key <= '9') {
                        current_selection = key - '1';  // Map key '1'-'9' to indices 0-8.
                        // Build a message such as "ChU1", "ChU2", etc.
                        char msg[5] = "CdU";
                        msg[3] = key; 
                        msg[4] = '\0';
                        display_message(msg);
                        clear_input();


                        ThisThread::sleep_for(1000ms);
                        slcd.clear(); //Clear "CdU "
                        display_message("C1d2");
                        clear_input();
                        key = '\0';

                        char confirm_key = '\0';
                        while (confirm_key != '#') {
                            key = keypad.ReadKey();  
                            if (key == '1' || key == '2') {
                                confirm_key = key;
                                slcd.clear();
                                slcd.Home();
                                slcd.printf("%c",confirm_key);
                            }
                            else if (key == '#') {
                                break;
                            }
                            ThisThread::sleep_for(200ms);
                        }
                        if(confirm_key == '1'){
                            slcd.clear();
                            slcd.Home();
                            clear_input();
                            slcd.printf("EnEr");
                            state = STATE_SET_NEW_PASSWORD;
                        }
                        else if(confirm_key == '2'){
                            state = STATE_DELETE_OLD_PASSWORD;
                        }
                    }
                    // Ignore any other keys.
                break;//end: STATE_ADMIN_MENU
                //----- STATE: DELETE_OLD_PASSWORD (Admin delete the old password) -----
                case STATE_DELETE_OLD_PASSWORD:
                    if (current_selection > -1 && current_selection < 9) {
                        memset(user_passwords[current_selection], 0, sizeof(user_passwords[current_selection]));
                        display_message("8888");
                        blink_led(2, 'g', 300ms);
                        // Return to normal mode.
                        state = STATE_NORMAL;
                        save_all_passwords_to_flash();
                        clear_input();
                        ThisThread::sleep_for(1000ms);
                        slcd.clear();
                    }
                break;//end STATE_DELETE_OLD_PASSWORD
                //----- STATE: SET_NEW_PASSWORD (Admin enters a new password) -----
                case STATE_SET_NEW_PASSWORD:
                    // Admin enters new password digits.
                    if (key != '#' && position < 8) {
                        entered_password[position] = key;
                        position++;
                        update_input_display();
                    }
                    else if (key == '#' && position > 0) {
                        entered_password[position] = '\0';
                        
                        // Ensure new password is at least 4 digits.
                        if (position < 4) {
                            display_message("INSF");
                            led_on_for('r', 1000ms);
                            clear_input();
                            continue;  // Stay in SET_NEW_PASSWORD state.
                        }
                        
                        // Pad with zeros if necessary.
                        if (position < 8) {
                            for (int i = position; i < 8; i++) {
                                entered_password[i] = '0';
                            }
                            entered_password[8] = '\0';
                        }
                        
                        // Save the new password according to selection.
                        if (current_selection == -1) {
                            // Update admin password.
                            strncpy(admin_password, entered_password, 8);
                            admin_password[8] = '\0';
                        } else {
                            // Update one of the nine user passwords.
                            strncpy(user_passwords[current_selection], entered_password, 8);
                            user_passwords[current_selection][8] = '\0';
                        }
                        
                        // Write all updated passwords to flash.
                        save_all_passwords_to_flash();
                        
                        // Confirmation message: "8888" indicates a successful save.
                        display_message("8888");
                        blink_led(2, 'g', 300ms);
                        
                        // Return to normal mode.
                        state = STATE_NORMAL;
                        clear_input();
                        ThisThread::sleep_for(1000ms);
                        slcd.clear();
                    }
                break;// end STATE_SET_NEW_PASSWORD
            }
        } // end if (key != NO_KEY)
        
        ThisThread::sleep_for(10ms);  // Polling delay.
    }
}