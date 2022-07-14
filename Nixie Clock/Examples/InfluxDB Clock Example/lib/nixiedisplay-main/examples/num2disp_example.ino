#include <NTC_PCA9698.h>
#include <Wire.h>
#include "num2disp.h"

static uint32_t num = 456789;
PCA9698 expanderChip0(0x20, 19, 18, 1000000); //(I2C_ADDR,SDA,SCL,SPEED)
PCA9698 expanderChip1(0x21, 19, 18, 1000000);


/* declare a NumericalDisplay_t for each tube */
NumericalDisplay_t tube1;
NumericalDisplay_t tube2;
NumericalDisplay_t tube3;
NumericalDisplay_t tube4;
NumericalDisplay_t tube5;
NumericalDisplay_t tube6;

/* pinout arrays to be associated with a NumericalDisplay_t */
uint8_t pinout1[10] = {9, 0, 1, 2, 3, 4, 5, 6, 7, 8};
uint8_t pinout2[10] = {27, 10, 11, 12, 13, 14, 15, 24, 25, 26};
uint8_t pinout3[10] = {39, 28, 29, 30, 31, 34, 35, 36, 37, 38};
uint8_t pinout4[10] = {49, 40, 41, 42, 43, 44, 45, 46, 47, 48};
uint8_t pinout5[10] = {67, 50, 51, 52, 53, 54, 55, 64, 65, 66};
uint8_t pinout6[10] = {79, 68, 69, 70, 71, 74, 75, 76, 77, 78};



void setup() {
  Serial.begin(115200);
  Wire.begin(19, 18, 100000);
  expanderChip0.configuration();
  expanderChip0.portMode(0, OUTPUT);
  expanderChip0.portMode(1, OUTPUT);
  expanderChip0.portMode(2, OUTPUT);
  expanderChip0.portMode(3, OUTPUT);
  expanderChip0.portMode(4, OUTPUT);

  
  expanderChip1.configuration();
  expanderChip1.portMode(0, OUTPUT);
  expanderChip1.portMode(1, OUTPUT);
  expanderChip1.portMode(2, OUTPUT);
  expanderChip1.portMode(3, OUTPUT);
  expanderChip1.portMode(4, OUTPUT);
for(int i = 0 ; i < 40; i++) {
    expanderChip0.digitalWrite(i, 0);
    expanderChip1.digitalWrite(i, 0);
  }

  /* link pinout array and NumericalDisplay_t */
  num2disp_createInstanceNumericalDisplay(&tube1, pinout1);
  num2disp_createInstanceNumericalDisplay(&tube2, pinout2);
  num2disp_createInstanceNumericalDisplay(&tube3, pinout3);
  num2disp_createInstanceNumericalDisplay(&tube4, pinout4);
  num2disp_createInstanceNumericalDisplay(&tube5, pinout5);
  num2disp_createInstanceNumericalDisplay(&tube6, pinout6);

  /* link all instances of NumericalDisplay_t */
  num2disp_createInstanceFullDisplay(&tube1, &tube2, &tube3, &tube4, &tube5, &tube6, 6, 0);

Serial.println(num2disp_runCathodePoisoningProtection(15, CATHODE_PROTECTION_STYLE_WAVE));

}

void loop() {

  /* write number to all NumericalDisplay_t in FullDisplayStruct */
  Serial.println(num2disp_writeNumberToFullDisplay(num, num - 1, true));
  num++;
 
  delay(500);
  

  
}


/* platform gpio write (i.e generic digitalwrite or write for other peripheral expander device */
bool num2disp_gpio_write(uint8_t pin, bool data) {
  if (pin >= 40) {
    pin -= 40;
    expanderChip1.digitalWrite(pin, data);
  }
  else {
    expanderChip0.digitalWrite(pin, data);
  }
  return false;
}
