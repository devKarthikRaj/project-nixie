/*
  getNixieExpanderPin.h - Custom circuit specific library that maps Nixie tube digits to the port expander pins
  they are connected to for ease of driving the tubes 
  Created by Karthik Raj, May 2, 2021.
  Released into the public domain.
*/

#include "Arduino.h"
#include "getNixieExpanderPin.h"

//Constructor
getNixieExpanderPin::getNixieExpanderPin()
{
}

//Method Definitions
//------------------
int getNixieExpanderPin::getPinNumber(int tube, int bcdDigit)
{
  _tube = tube;
  _bcdDigit = bcdDigit;
  if(_tube == 1)
  {
    switch(_bcdDigit)
    {
      case 1:
        _pin = 0;
        break;
      case 2:
        _pin = 1;
        break;
      case 3:
        _pin = 2;
        break;
      case 4:
        _pin = 3;
        break;
      case 5:
        _pin = 4;
        break;
      case 6:
        _pin = 5;
        break;
      case 7:
        _pin = 6;
        break;
      case 8:
        _pin = 7;
        break;
      case 9:
        _pin = 8;
        break;
      case 0:
        _pin = 9;
        break;
    }
  }

  if(_tube == 2)
  {
    switch(_bcdDigit)
    {
      case 1:
        _pin = 10;
        break;
      case 2:
        _pin = 11;
        break;
      case 3:
        _pin = 12;
        break;
      case 4:
        _pin = 13;
        break;
      case 5:
        _pin = 14;
        break;
      case 6:
        _pin = 15;
        break;
      case 7:
        _pin = 24;
        break;
      case 8:
        _pin = 25;
        break;
      case 9:
        _pin = 26;
        break;
      case 0:
        _pin = 27;
        break;
    }
  }

  if(_tube == 3)
  {
    switch(_bcdDigit)
    {
      case 1:
        _pin = 28;
        break;
      case 2:
        _pin = 29;
        break;
      case 3:
        _pin = 30;
        break;
      case 4:
        _pin = 31;
        break;
      case 5:
        _pin = 34;
        break;
      case 6:
        _pin = 35;
        break;
      case 7:
        _pin = 36;
        break;
      case 8:
        _pin = 37;
        break;
      case 9:
        _pin = 38;
        break;
      case 0:
        _pin = 39;
        break;
    }
  }

  if(_tube == 4)
  {
    switch(_bcdDigit)
    {
      case 1:
        _pin = 0;
        break;
      case 2:
        _pin = 1;
        break;
      case 3:
        _pin = 2;
        break;
      case 4:
        _pin = 3;
        break;
      case 5:
        _pin = 4;
        break;
      case 6:
        _pin = 5;
        break;
      case 7:
        _pin = 6;
        break;
      case 8:
        _pin = 7;
        break;
      case 9:
        _pin = 8;
        break;
      case 0:
        _pin = 9;
        break;
    }
  }

  if(_tube == 5)
  {
    switch(_bcdDigit)
    {
      case 1:
        _pin = 10;
        break;
      case 2:
        _pin = 11;
        break;
      case 3:
        _pin = 12;
        break;
      case 4:
        _pin = 13;
        break;
      case 5:
        _pin = 14;
        break;
      case 6:
        _pin = 15;
        break;
      case 7:
        _pin = 24;
        break;
      case 8:
        _pin = 25;
        break;
      case 9:
        _pin = 26;
        break;
      case 0:
        _pin = 27;
        break;
    }
  }

  if(_tube == 6)
  {
    switch(_bcdDigit)
    {
      case 1:
        _pin = 28;
        break;
      case 2:
        _pin = 29;
        break;
      case 3:
        _pin = 30;
        break;
      case 4:
        _pin = 31;
        break;
      case 5:
        _pin = 34;
        break;
      case 6:
        _pin = 35;
        break;
      case 7:
        _pin = 36;
        break;
      case 8:
        _pin = 37;
        break;
      case 9:
        _pin = 38;
        break;
      case 0:
        _pin = 39;
        break;
    }
  }
  return _pin;
}












