# num2disp

Arduino library for interfacing GPIO to generic nixie tubes or other numerical displays with discrete 0-9 control pins. 

## Prerequisites
* Arduino compatible microcontroller
* Sufficient microcontroller GPIO, or GPIO expander



## How to use
\
Declare an instance of NumericalDisplay_t and pinout array for each tube. The pinout must be ordered from 0 to 9.
```C++
NumericalDisplay_t tube1;
uint8_t pinout1[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
```
\
Link the pinout to the NumericalDisplay_t.
```C++
num2disp_createInstanceNumericalDisplay(&tube1, pinout1);
```
\
Link all the instances of NumericalDisplay_t to be controlled as a single set. The last two arguments are the number of active tubes (of 6) and right shift respectively.
```C++
num2disp_createInstanceFullDisplay(&tube1, &tube2, &tube3, &tube4, &tube5, &tube6, 6, 0);
```
\
Write a number to the tubes. The first two arguments are the number to be written and the previous number (i.e the one currently on the tubes). The final argument is an option to enable crossfade transition.
```C++
uint32_t x = 123456;
num2disp_writeNumberToFullDisplay(x, x - 1, true);
```
\
Add this function to the .ino file. It must contain a method for writing to GPIO. Return false if no error.
```C++
bool num2disp_gpio_write(uint8_t pin, bool data) {
    digitalWrite(pin, data); // write to GPIO
    
    xxxx.write(pin, data) // write to GPIO expander or other IO device
    return false;
}
```
\
Run a cathode protection routine. Pass the number of iterations to cycle through 0-9, then select style CATHODE_PROTECTION_STYLE_WAVE or CATHODE_PROTECTION_STYLE_SLOT for different display appearances. Warning: this is a blocking function for the duration of the iterations.
```C++
num2disp_runCathodePoisoningProtection(50, CATHODE_PROTECTION_STYLE_WAVE );
```
