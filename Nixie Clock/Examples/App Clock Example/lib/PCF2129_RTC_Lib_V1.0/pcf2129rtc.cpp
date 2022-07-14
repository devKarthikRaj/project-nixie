/*
  PCF2129rtc.cpp - Library for interfacing with NXP Semiconductor's PCF2129 Real Time Clock Chip.
  Created by Karthik Raj, April 12, 2021.
  Released into the public domain.
*/

#include "Arduino.h"
#include "pcf2129rtc.h"
#include "Wire.h"

//Constructor
pcf2129rtc::pcf2129rtc(int sda, int scl)
{
  _sda = sda;
  _scl = scl;

  //Initialize I2C - initiate wire library and join I2C bus as master
  Wire.begin(_sda, _scl); 
}

//Method Definitions
//------------------
void pcf2129rtc::rtcInitialConfig()
{
  //RTC initial config 
  //Control reg 1 config
  Wire.beginTransmission(0x51); //Start + write slave address
  Wire.write(0x00); //Write reg address
  Wire.write(0x01); //Write data
  Wire.endTransmission();
  
  //Control reg 2 config
  Wire.beginTransmission(0x51); 
  Wire.write(0x01); 
  Wire.write(0x00); 
  Wire.endTransmission();

  //Control reg 3 config
  Wire.beginTransmission(0x51);
  Wire.write(0x02); 
  Wire.write(0xE0); 
  Wire.endTransmission();

  //Watchdg_tim_ctl
  Wire.beginTransmission(0x51);
  Wire.write(0x10);
  Wire.write(0x20);
  Wire.endTransmission();
}

void pcf2129rtc::updateCurrentTimeToRTC(int initHour, int initMin, int initSec)
{
  _initHour = initHour;
  _initMin = initMin;
  _initSec = initSec;

  //Update seconds register in RTC
  Wire.beginTransmission(0x51);
  Wire.write(0x03);
  _bcdSec = pcf2129rtc::decToBcd(_initSec); //convert _initSec to BCD (RTC works in BCD)
  Wire.write(_bcdSec); 
  Wire.endTransmission();

  //Update minutes register in RTC
  Wire.beginTransmission(0x51);
  Wire.write(0x04);
  _bcdMin = pcf2129rtc::decToBcd(_initMin); //convert _initMin to BCD (RTC works in BCD)
  Wire.write(_bcdMin); 
  Wire.endTransmission();

  //Update hours register in RTC
  Wire.beginTransmission(0x51);
  Wire.write(0x05);
  _bcdHour = pcf2129rtc::decToBcd(_initHour); //convert _initHour to BCD (RTC works in BCD)
  Wire.write(_bcdHour);
  Wire.endTransmission();
}

int pcf2129rtc::readRtcHour()
{
  //Read hour from RTC
  Wire.beginTransmission(0x51);
  Wire.write(0x05);
  Wire.endTransmission();
  Wire.requestFrom(0x51,1);
  if(Wire.available()) {
    _rtcHour = (Wire.read());
  }
  return _rtcHour;
}

int pcf2129rtc::readRtcMin()
{
  //Read minute from RTC
  Wire.beginTransmission(0x51);
  Wire.write(0x04);
  Wire.endTransmission();
  Wire.requestFrom(0x51,1);
  if(Wire.available()) {
    _rtcMin = (Wire.read());
  }
  return _rtcMin;
}

int pcf2129rtc::readRtcSec()
{
  //Read second from RTC
  Wire.beginTransmission(0x51);
  Wire.write(0x03);
  Wire.endTransmission();
  Wire.requestFrom(0x51,1);
  if(Wire.available()) {
    _rtcSec = (Wire.read());
  }
  return _rtcSec;
}

int pcf2129rtc::clearMsf()
{
  //Control reg 2 confing
  Wire.beginTransmission(0x51); 
  Wire.write(0x01); 
  Wire.write(0x00); 
  Wire.endTransmission();
}

int pcf2129rtc::readRtcSecBCD0()
{
  int _rtcSecInt = pcf2129rtc::readRtcSec();
  bitWrite(_rtcSecBCD0,0,bitRead(_rtcSecInt,0));
  bitWrite(_rtcSecBCD0,1,bitRead(_rtcSecInt,1));
  bitWrite(_rtcSecBCD0,2,bitRead(_rtcSecInt,2));
  bitWrite(_rtcSecBCD0,3,bitRead(_rtcSecInt,3));

  return _rtcSecBCD0;
}

int pcf2129rtc::readRtcSecBCD1()
{
  int _rtcSecInt = pcf2129rtc::readRtcSec();
  bitWrite(_rtcSecBCD1,0,bitRead(_rtcSecInt,4));
  bitWrite(_rtcSecBCD1,1,bitRead(_rtcSecInt,5));
  bitWrite(_rtcSecBCD1,2,bitRead(_rtcSecInt,6));
  bitWrite(_rtcSecBCD1,3,bitRead(_rtcSecInt,7));

  return _rtcSecBCD1;
}

int pcf2129rtc::readRtcMinBCD0()
{
  int _rtcMinInt = pcf2129rtc::readRtcMin();
  bitWrite(_rtcMinBCD0,0,bitRead(_rtcMinInt,0));
  bitWrite(_rtcMinBCD0,1,bitRead(_rtcMinInt,1));
  bitWrite(_rtcMinBCD0,2,bitRead(_rtcMinInt,2));
  bitWrite(_rtcMinBCD0,3,bitRead(_rtcMinInt,3));

  return _rtcMinBCD0;
}

int pcf2129rtc::readRtcMinBCD1()
{
  int _rtcMinInt = pcf2129rtc::readRtcMin();
  bitWrite(_rtcMinBCD1,0,bitRead(_rtcMinInt,4));
  bitWrite(_rtcMinBCD1,1,bitRead(_rtcMinInt,5));
  bitWrite(_rtcMinBCD1,2,bitRead(_rtcMinInt,6));
  bitWrite(_rtcMinBCD1,3,bitRead(_rtcMinInt,7));

  return _rtcMinBCD1;
}

int pcf2129rtc::readRtcHourBCD0()
{
  int _rtcHourInt = pcf2129rtc::readRtcHour();
  bitWrite(_rtcHourBCD0,0,bitRead(_rtcHourInt,0));
  bitWrite(_rtcHourBCD0,1,bitRead(_rtcHourInt,1));
  bitWrite(_rtcHourBCD0,2,bitRead(_rtcHourInt,2));
  bitWrite(_rtcHourBCD0,3,bitRead(_rtcHourInt,3));

  return _rtcHourBCD0;
}

int pcf2129rtc::readRtcHourBCD1()
{
  int _rtcHourInt = pcf2129rtc::readRtcHour();
  bitWrite(_rtcHourBCD1,0,bitRead(_rtcHourInt,4));
  bitWrite(_rtcHourBCD1,1,bitRead(_rtcHourInt,5));
  bitWrite(_rtcHourBCD1,2,bitRead(_rtcHourInt,6));
  bitWrite(_rtcHourBCD1,3,bitRead(_rtcHourInt,7));

  return _rtcHourBCD1;
}

int pcf2129rtc::decToBcd(int dec)
{
  _dec = dec;
  _bcd = _dec + ( (6) * ( (_dec - (_dec%10) ) / (10) ) );
  return _bcd;

}
















