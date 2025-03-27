#include "input_module.h"
#include <cstring>

InputModule::InputModule(Keypad& keypad, SLCD& slcd) : keypad(keypad), slcd(slcd) {
    PASSWORD_ENTERED = false;
    reset();
}

void InputModule::processInput() {
    char key = keypad.ReadKey();
    if (key != NO_KEY) 
    {
        if (position < 8 && key != '*' && key != '#') 
        {
            slcd.clear(); // Removes the Entr Message from screen
            slcd.Home();
            entered_password[8] = '\0';
            entered_password[position] = key;

            position++;

            // Always show last 4 characters from left to right
            char showed_password[9] = {0};
            if (position <= 4) {
                // If less than 4 characters entered, show all entered characters
                strncpy(showed_password, entered_password, position);
            } else {
                // If more than 4 characters entered, show last 4 from left to right
                strncpy(showed_password, entered_password + (position - 4), 4);
            }

            slcd.printf("%s", showed_password);
        }

        else if (key == '*') // Clear
        {
            reset();
        }

        else if (key == '#') //Confirm
        {
            slcd.clear();
            slcd.Home();
            entered_password[8] = '\0'; // Ensure null termination
            
            // Check if password is too short
            if (position < 8) {
                slcd.printf("INSF");  // Display "INSF"
                ThisThread::sleep_for(2s);
                reset();
            }

            else {
                PASSWORD_ENTERED = true;
            }
        }
    }
}

void InputModule::reset() {
    PASSWORD_ENTERED = false;
    position = 0;
    memset(entered_password, 0, sizeof(entered_password));
    memset(showed_password, 0, sizeof(showed_password));
    slcd.clear();
    slcd.Home();
    slcd.printf("Entr");
    ThisThread::sleep_for(500ms);
}