/*
  getNixieExpanderPin.h - Custom circuit specific library that maps Nixie tube digits to the port expander pins
  they are connected to for ease of driving the tubes 
  Created by Karthik Raj, May 2, 2021.
  Released into the public domain.
*/

#ifndef getNixieExpanderPin_h 
#define getNixieExpanderPin_h

#include "Arduino.h"

class getNixieExpanderPin
{
  public:
    //Constructor
    getNixieExpanderPin();

    int getPinNumber(int tube, int bcdDigit);

  private:
    int _tube;
    int _bcdDigit;
    int _pin;
};

#endif