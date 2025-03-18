#pragma once
#undef __ARM_FP // removes the error on the #include "mbed.h" line
#include "mbed.h"


class Keypad
{
    public:

    // constructor for the keypad class - you need to supply the pin names to which the rows/
    // and columns are connected
    Keypad(PinName row0, 
           PinName row1, 
           PinName row2, 
           PinName row3, 
           PinName col0, 
           PinName col1, 
           PinName col2);

    // function that reads a key - returns mmediately with the key pressed or NO_KEY
    // only one instance of a key value will be returned - coincident with the onset of 
    // the key being pressed. Only publicly available member function
    char ReadKey(void);

   // value that means no key is pressed
   #define NO_KEY 0

    protected:

    private:

    // this key scan function does most of the heavy lifting it is intended to be spawned as 
    // an independent thread that runs in the background
    void KeyScanner(void);

    // thread to run the key scanner every 10ms
    Thread keyscan;

    // class definitions for the rwos (inputs) and columns (outputs)
    // however these objects are not fully initialised (i.e. the In/Outs
    // have not been associated with pins. This is done when the class is
    // initialised - see the associations after the colon in ketypad::keypad )
    DigitalIn _row0, _row1, _row2, _row3;
    DigitalOut _col0, _col1, _col2;

    // keypad mapping key position to value
    // this returns printable ascii codes corresponding to each key
    char mapping[3][4] = {{'1','4','7','#'},
                          {'2','5','8','0'},
                          {'3','6','9','*'}};

    // variables shared between keyscanner and read key
    char key, key_p;

};