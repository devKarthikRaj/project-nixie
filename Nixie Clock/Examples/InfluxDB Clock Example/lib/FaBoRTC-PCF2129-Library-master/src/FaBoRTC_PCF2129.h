/**
 @file FaBoRTC_PCF2129.h
 @brief This is a library for the FaBo RTC I2C Brick.

   http://fabo.io/215.html

   Released under APACHE LICENSE, VERSION 2.0

   http://www.apache.org/licenses/

 @author FaBo<info@fabo.io>
*/

#ifndef FABORTC_PCF2129_H
#define FABORTC_PCF2129_H

#include <Arduino.h>
#include <Wire.h>

#define PCF2129_SLAVE_ADDRESS 0x51 ///< I2C Slave Address

/// @name Register Addresses
/// @{
#define PCF2129_CONTROL_REGISTERS 0x00
#define PCF2129_CONTROL_12_24 0x04
#define PCF2129_SECONDS 0x03
#define PCF2129_MINUTES 0x04
#define PCF2129_HOURS 0x05
#define PCF2129_DAYS 0x06
#define PCF2129_WEEKDAYS 0x07
#define PCF2129_MONTHS 0x08
#define PCF2129_YEARS 0x09
/// @}

/**
 @class DateTime
 @brief RTC DateTime class
*/
class DateTime {
  public:
    DateTime (uint16_t years, uint8_t months, uint8_t days,
              uint8_t hours, uint8_t minutes, uint8_t seconds);
    uint16_t year()  const { return y+2000; }
    uint8_t month()  const { return m; }
    uint8_t day()    const { return d; }
    uint8_t hour()   const { return hh; }
    uint8_t minute() const { return mm; }
    uint8_t second() const { return ss; }
  protected:
    uint8_t y,m,d,hh,mm,ss;
};

/**
 @class FaBoRTC_PCF2129
 @brief FaBo RTC I2C Controll class
*/
class FaBoRTC_PCF2129 {
  public:
    FaBoRTC_PCF2129(uint8_t addr = PCF2129_SLAVE_ADDRESS);
    bool searchDevice(void);
    void configure(void);
    uint8_t getSeconds(void);
    void setSeconds(uint8_t seconds);
    uint8_t getMinutes(void);
    void setMinutes(uint8_t minutes);
    uint8_t getHours(void);
    void setHours(uint8_t hours);
    uint8_t getDays(void);
    void setDays(uint8_t days);
    uint8_t getWeekdays(void);
    void setWeekdays(uint8_t weekdays);
    uint8_t getMonths(void);
    void setMonths(uint8_t months);
    uint8_t getYears(void);
    void setYears(uint8_t years);
    DateTime now(void);
    void setDate(uint16_t years, uint8_t months, uint8_t days,
                 uint8_t hours, uint8_t minutes, uint8_t seconds);
    void set12mode(void);
    void set24mode(void);
  private:
    uint8_t _i2caddr;
    uint8_t bcdToDec(uint8_t value);
    uint8_t decToBcd(uint8_t value);
    uint8_t readI2c(uint8_t address);
    void writeI2c(uint8_t address, uint8_t data);
    uint8_t readCtrl(void);
    void writeCtrl(uint8_t data);
};

#endif // FABORTC_PCF2129_H