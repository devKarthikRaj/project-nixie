/*
  PCF2129rtc.h - Library for interfacing with NXP Semiconductor's PCF2129 Real Time Clock Chip.
  Created by Karthik Raj, April 12, 2021.
  Released into the public domain.
*/

#ifndef pcf2129rtc_h 
#define pcf2129rtc_h

#include "Arduino.h"
#include "Wire.h" 

class pcf2129rtc
{
  public:
    //Constructor
    pcf2129rtc(int sda, int scl);

    //RTC initial config
    void rtcInitialConfig();
            
    //Write current real world time to RTC
    void updateCurrentTimeToRTC(int initHour, int initMin, int initSec);  
    
    //Reset interrupt
    int clearMsf();

    //Read time as integer (Needs to be converted to BCD to make sense as the chip operates in BCD)
    int readRtcHour();
    int readRtcMin();
    int readRtcSec();

    //Read time in BCD 
    int readRtcHourBCD0();
    int readRtcHourBCD1();
    int readRtcMinBCD0();
    int readRtcMinBCD1();
    int readRtcSecBCD0();
    int readRtcSecBCD1();

  private:
    //I2C Vars
    int _sda;
    int _scl;

    //RTC Write Time Vars in decimal
    int _initHour;
    int _initMin;
    int _initSec;

    //RTC Write Time Vars in BCD
    int _bcdHour;
    int _bcdMin;
    int _bcdSec;

    //RTC Read Time Vars
    //Time in Integer (need to be converted to BCD to make sense)
    int _rtcHour;
    int _rtcMin;
    int _rtcSec;

    //Time in BCD (ex: 56 = 0101 0110)
    //                      BCD1 BCD0
    int _rtcSecBCD0;
    int _rtcSecBCD1;
    int _rtcMinBCD0;
    int _rtcMinBCD1;
    int _rtcHourBCD0;
    int _rtcHourBCD1; 

    //Convert decimal integer to BCD integer function
    int decToBcd(int dec);
        //decToBcd() internal Vars
        int _dec;
        int _bcd;

};

#endif