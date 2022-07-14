# NixieDisplay HAL

C++ abstraction for controlling up to 6 nixie tubes. Compatible with most platforms that provide standard GPIO write and delay/sleep functions. Put the c++ and h files in a folder called "src" to use with Arduino.

## Prerequisites
* Any modern microcontroller 
* Sufficient GPIO to interface with the tubes, or GPIO expander.


## How to use
\
Declare uint8_t arrays pinout1 through pinout6, containing the control pins for each digit ordered from 0 to 9.
```C++
uint8_t pinout1[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
uint8_t pinout2[10] = {....}
...
...
uint8_t pinout6[10] = {....}
```
\
Instantiate the NixieDisplay object.
```C++
NixieDisplay display(uint8_t active, uint8_t offset, 
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
Or write time to the display.
```C++
display.writeTime(struct tm *time);
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
Option to enable/disable scrollback of digits between write() calls. Defaults to true.
```C++
display.setScrollback(bool scrollback)
```
\
Run cathode protection type nixie_display_protection_t for a duration of ms (minimum duration is CATHODE_PROTECTION_INTER_MS*10). Optional parameter CATHODE_PROTECTION_INTER_MS to determine on time of each digit.
```C++
display.runProtection(nixie_display_protection_t type, uint32_t ms, uint32_t CATHODE_PROTECTION_INTER_MS = 15);
```
\
Add this function to the main.c file. It must contain a platform-specific method for writing to GPIO. Return false if no error.
```C++
bool platformGPIOWrite(uint8_t pin, bool data) {
    xxxx.write(pin, data) // write to GPIO expander or other IO device
    return false;
}
```
\
Add this function to the main.c file. It must contain a platform-specific method for milisecond delays.
```C++
void platformDelayMs(uint32_t ms){
  vTaskDelay(ms);
}

```
