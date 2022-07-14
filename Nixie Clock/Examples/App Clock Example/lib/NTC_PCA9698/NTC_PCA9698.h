/**
 @file NTC_PCA9698.c
 @brief Driver for I2C communication with PCA9698
 @author Edward62740
*/

#ifndef NTC_PCA9698_H
#define NTC_PCA9698_H

#include <Arduino.h>
#include <Wire.h>

#define PCA9698_OUTPUT_REG 0x01
#define PCA9698_CONFIGURATION_REG 0x03
#define PCA9698_INPUT_PORT0 0x00
#define PCA9698_INPUT_PORT1 0x01
#define PCA9698_INPUT_PORT2 0x02
#define PCA9698_INPUT_PORT3 0x03
#define PCA9698_INPUT_PORT4 0x04
#define PCA9698_OUTPUT_PORT0 0x08
#define PCA9698_OUTPUT_PORT1 0x09
#define PCA9698_OUTPUT_PORT2 0x0A
#define PCA9698_OUTPUT_PORT3 0x0B
#define PCA9698_OUTPUT_PORT4 0x0C
#define PCA9698_CONFIG_PORT0 0x18
#define PCA9698_CONFIG_PORT1 0x19
#define PCA9698_CONFIG_PORT2 0x1A
#define PCA9698_CONFIG_PORT3 0x1B
#define PCA9698_CONFIG_PORT4 0x1C

/**
 @class PCA9698
 @brief PCA9698 I2C control class
*/
class PCA9698 {
  public:
    PCA9698(uint8_t addr, int pin_SDA, int pin_SCL, uint32_t bus_speed);
    void configuration(void);
    void digitalWrite(uint8_t pin, uint8_t output);
    int digitalRead(uint8_t pin);
    void setAllClear();
    void portMode(uint8_t port, uint8_t mode);
  private:
    uint8_t _i2caddr;
    uint8_t _output_port0;
    uint8_t _output_port1;
    uint8_t _output_port2;
    uint8_t _output_port3;
    uint8_t _output_port4;
    void writeI2c(uint8_t address, uint8_t data);
    void readI2c(uint8_t address, uint8_t num, uint8_t * data);
};

#endif // NTC_PCA9698_H
