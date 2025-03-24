#ifndef INPUT_MODULE_H
#define INPUT_MODULE_H

#undef __ARM_FP // removes the error on the #include "mbed.h" line
#include "mbed.h"
#include "keypad.h"
#include "SLCD/SLCD.h"

class InputModule {
private:
    Keypad& keypad;
    SLCD& slcd;
    char showed_password[9] = {'\0'};
    char entered_password[9] = {'\0'}; 
    int position = 0;
    bool PASSWORD_ENTERED;

public:
    InputModule(Keypad& keypad, SLCD& slcd);
    void processInput();
    void reset();
    bool hasPassword() const { return PASSWORD_ENTERED; }
    const char* getEnteredPassword() const { return entered_password; }
};

#endif // INPUT_MODULE_H
