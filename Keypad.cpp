#include "keypad.h"
#include "ThisThread.h"
#include <iostream>


// keypad constructor - allows the pins for the rows/columns to be specified
// notice the information after the single colon. This in member initialisation
// for objects defined in Keypad.h
// the keypad constructor is run when an object of this class is created. It performas
// any setup required
Keypad::Keypad(PinName row0, 
               PinName row1, 
               PinName row2, 
               PinName row3, 
               PinName col0, 
               PinName col1, 
               PinName col2): _row0(row0), _row1(row1), _row2(row2), _row3(row3), 
                              _col0(col0), _col1(col1), _col2(col2)
{

    // create a 10ms scanning facility using a thread
    // the thread keyscan is also defined in Keypad.h
    // the callback() construct seems odd but is required
    // to ensure that the function holding the thread is
    // a function that is part of this specific class and object
    // KeyScanner scans the keys and assigns the instantaneous 
    // key press to key
    keyscan.start(callback(this, &Keypad::KeyScanner));

    // initialise key to NO_KEY (nothing pressed)
    key = NO_KEY;

    // key_p is the value last time you interrogated the keypad
    key_p = NO_KEY;
}


// key scanning thread
// apply a 0 to one column read all the rows.
// repeat for all columns
// very simple - assumes only one key pressed
// could be improved
void Keypad::KeyScanner(void)
{ // use a local variable and only assign key at end
  char k;
    // do forever. This to ensure that the thread never finishes
    while (1==1) {

        // set key to NO_KEY
        k = NO_KEY;

        // left column
        _col0 = 0; _col1 = 1; _col2 = 1;
        // read rows
        if (_row0 == 0) k = mapping[0][0];
        if (_row1 == 0) k = mapping[0][1];
        if (_row2 == 0) k = mapping[0][2];
        if (_row3 == 0) k = mapping[0][3];

        // middle column
        _col0 = 1; _col1 = 0; _col2 = 1;
        // read rows
        if (_row0 == 0) k = mapping[1][0];
        if (_row1 == 0) k = mapping[1][1];
        if (_row2 == 0) k = mapping[1][2];
        if (_row3 == 0) k = mapping[1][3];

        // right column
        _col0 = 1; _col1 = 1; _col2 = 0;
        // read rows
        if (_row0 == 0) k = mapping[2][0];
        if (_row1 == 0) k = mapping[2][1];
        if (_row2 == 0) k = mapping[2][2];
        if (_row3 == 0) k = mapping[2][3];

        // finally assign k to key - if no key press found
        // it will be NO_KEY
        // this seems unnecessary but we cannto be sure that the function
        // will run without being interrupted by another thread and if this 
        // were to happen then intermediate values (such as when  k = NO_KEY)
        // might be seen elsewhere in the program. This is a fundamental problem
        // of interrupt and normal code sharing/manipulating the same data - 
        // access needs to be controlled carefully. The word is 'atomic'
        key = k;

        // go to sleep for 10ms then loop back and read the keys again
        // 10ms is a sweet spot: it's short enough so a uset thinks the 
        // response is instantaneous but longer than the key bounce period
        // so we should not encounter any issues there
        ThisThread::sleep_for(10ms);
    }
}

// uses current value of key to determine if key pressed
// this is the only puplic function of the class and the 
// only one that can be used - everything else happens under
// the hood
char Keypad::ReadKey(void)
{
    // only do something if the current value of the key
    // is different to the last time this function was called
    // this is the instant a key is pressed or de-pressed
    if (key != key_p) {

        // update the previous key value (for next time)
        key_p = key;

        // return key - the only time a value other than NO_KEY
        // is returned is when a key has just been pressed
        return key;
    }

    // by default return NO_KEY
    return NO_KEY;
}
