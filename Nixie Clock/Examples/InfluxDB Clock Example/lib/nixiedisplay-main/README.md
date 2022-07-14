# num2disp

Arduino library for controlling up to 6 nixie tubes with standard display functions.

## Prerequisites
* Arduino compatible microcontroller
* Sufficient microcontroller GPIO, or GPIO expander



## How to use
\
Declare uint8_t arrays pinout1 through pinout6, containing the control pins for each digit ordered from 0 to 9.
```C++
NumericalDisplay_t tube1;
uint8_t pinout1[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
uint8_t pinout2[10] = {....}
...
...
uint8_t pinout6[10] = {....}
```
\
Instantiate the NixieDisplay object.
```C++
    NixieDisplay display(uint8_t active, uint8_t ctrl, 
    uint8_t pinout1[], uint8_t pinout2[], uint8_t pinout3[], uint8_t pinout4[], uint8_t pinout5[], uint8_t pinout6[]);
```
\
Initialize display.
```C++
display.init();
```
\

Write a number to the display.
```C++
display.write(uint32_t num);
```
\
Clear the display. This blanks the display and may not have the same effect as disabling the nixie tube's power supply.
```C++
display.clear();
```
\
Option to enable/disable crossfading of digits between write() calls. Defaults to true.
```C++
display.setCrossfade(bool crossfade)
```
\
Run cathode protection type nixie_display_protection_t for a duration of ms (minimum duration is CATHODE_PROTECTION_INTER_MS*10). Optional parameter CATHODE_PROTECTION_INTER_MS to determine on time of each digit.
```C++
display.runProtection(nixie_display_protection_t type, uint32_t ms, uint32_t CATHODE_PROTECTION_INTER_MS = 15);
```
\

Add this function to the .ino file. It must contain a method for writing to GPIO. Return false if no error.
```C++
bool platformGPIOWrite(uint8_t pin, bool data) {
    digitalWrite(pin, data); // write to GPIO
    
    xxxx.write(pin, data) // write to GPIO expander or other IO device
    return false;
}
```