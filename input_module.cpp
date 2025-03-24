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
            memset(showed_password, ' ', 8);
            entered_password[8] = '\0';
            entered_password[position] = key;

            for(int i = 0; i < position; i++)
            {
                showed_password[7 - position + i] = entered_password[i];
            }

            position++;
            showed_password[8] = '\0'; // Ensure null termination

            slcd.clear();
            slcd.Home();
            slcd.printf("%s", &showed_password[4]);
        }

        else if (key == '*') // Clear
        {
            reset();
        }

        else if (key == '#') //Confirm
        {
            entered_password[8] = '\0'; // Ensure null termination
            
            // Check if password is too short
            if (position < 8) {
                slcd.printf("FAIL");  // Display "FAIL"
                PASSWORD_ENTERED = false;
                reset();
            }

            else {
                PASSWORD_ENTERED = true;
                slcd.clear();
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
    ThisThread::sleep_for(500ms);
}