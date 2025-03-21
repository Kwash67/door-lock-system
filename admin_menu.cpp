#undef __ARM_FP // removes the error on the #include "mbed.h" line
#include "mbed.h"
#include "SLCD.h"
#include "keypad.h"
#include "admin_menu.h"

// Forward declarations for functions used in the menu system
void displayMenu();
void handleKeypress(char key);
void executeMenuItem(int menuIndex);

// Menu items
const char* menuItems[] = {
    "1.USER PASS",
    "2.ADD USER",
    "3.DEL USER",
    "4.ADMIN PASS"
};

const int numMenuItems = 4;
int currentMenuItem = 0;
bool menuActive = true;

// Timer for scrolling text
Ticker scrollTicker;
/** A Ticker is used to call a function at a recurring interval
 *
 *  You can use as many separate Ticker objects as you require.
 *
 * @note Synchronization level: Interrupt safe
 */
int scrollPosition = 0;


// Reference to SLCD from main.cpp
SLCD* slcdPtr = NULL;

void displayMenu() {
    if (!menuActive || slcdPtr == NULL) return;
    
    const char* text = menuItems[currentMenuItem];
    int textLength = strlen(text);
    
    // Clear the display first
    slcdPtr->clear();
    
    // If text is 4 characters or less, just display it without scrolling
    if (textLength <= 4) {
        slcdPtr->printf("%s", text);
        return;
    }
    
    // Display 4 characters starting from scrollPosition
    char buffer[5]; // 4 characters + null terminator
    for (int i = 0; i < 4; i++) {
        if (scrollPosition + i < textLength) {
            buffer[i] = text[scrollPosition + i]; // buffer is text offset by scrollPosition
        } else {
            buffer[i] = ' '; // Pad with spaces if we run out of text
        }
    }
    buffer[4] = '\0'; // Null-terminate the string
    
    slcdPtr->printf("%s", buffer);
    
    // Update scroll position for next time
    scrollPosition++;
    
    // If we've scrolled past the end, start new item
    if (scrollPosition > textLength) {
        scrollPosition = 0;
    }
}

void nextMenuItem() {
    currentMenuItem = (currentMenuItem + 1) % numMenuItems;
    scrollPosition = 0; // Reset scroll position for new item
}

void executeMenuItem(int menuIndex) {
    menuActive = false;
    scrollTicker.detach(); // Stop scrolling
    
    // Handle menu selection
    switch (menuIndex) {
        case 0: // Set User Password
            slcdPtr->clear();
            slcdPtr->printf("USR");
            // Add user password setting functionality
            break;
        case 1: // Add User
            slcdPtr->clear();
            slcdPtr->printf("ADD");
            // Add new user functionality
            break;
        case 2: // Remove User
            slcdPtr->clear();
            slcdPtr->printf("DEL");
            // Delete user functionality
            break;
        case 3: // Set Admin Password
            slcdPtr->clear();
            slcdPtr->printf("ADM");
            // Admin password setting functionality
            break;
    }
    
    // After completing the action, return to menu
    wait_ms(2000); // Wait for 2 seconds to show the selection
    menuActive = true;
    scrollPosition = 0;
    scrollTicker.attach(&displayMenu, 0.5); // Restart scrolling
}

void handleKeypress(char key) {
    if (!menuActive) return;
    
    switch (key) {
        case '#': // Use # as "next" button
            nextMenuItem();
            scrollPosition = 0;
            break;
        case '*': // Use * as "select" button
            executeMenuItem(currentMenuItem);
            break;
        case '1':
            executeMenuItem(0);
            break;
        case '2':
            executeMenuItem(1);
            break;
        case '3':
            executeMenuItem(2);
            break;
        case '4':
            executeMenuItem(3);
            break;
        default:
            // Ignore other keypresses
            break;
    }
}

void initMenu() {
    // Initialize the LCD
    slcdPtr->Contrast(10); // Set contrast to a suitable level
    
    // Start the menu scrolling
    scrollTicker.attach(&displayMenu, 0.5); // Scroll every 0.5 seconds
}

// This function should be called from your main.cpp when a key is pressed
void processMenuKey(char key) {
    handleKeypress(key);
}