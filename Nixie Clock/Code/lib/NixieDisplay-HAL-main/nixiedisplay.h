/**
 @file nixiedisplay.h
 @brief Library for interfacing with nixie tubes
 @author Edward62740
 */

#ifndef NIXIEDISPLAY_H
#define NIXIEDISPLAY_H

#include <Arduino.h>
#include <Wire.h>
#include "time.h"


#define CROSSFADE_PULSE_CYCLE_MS 20
#define SCROLLBACK_INTER_MS 25

typedef struct TubeStruct
{
    uint8_t pinout[10];
    uint8_t index;
} TubeStruct_t;

struct DisplayStruct
{
    TubeStruct_t tube[6];
    uint8_t active;
    uint8_t offset;
    bool crossfade = true;
    bool scrollback = true;
};

typedef enum err_codes
{
    NO_ERR, // no error
	ERR_FAIL, // operation failed
	ERR_PARAM, // invalid parameter
	ERR_INT // internal failure
} nixie_display_err_t;

typedef enum protection_types
{
	CATHODE_PROTECTION_STYLE_WAVE, // drives 0123456789 through the tubes in order, wrapping over at the end
	CATHODE_PROTECTION_STYLE_SLOT, // cycles through 0-9 for all the tubes
	CATHODE_PROTECTION_STYLE_SEQUENTIAL // cycles through all digits based on physical position (for IN-14 tubes)
} nixie_display_protection_t;


extern bool platformGPIOWrite(uint8_t pin, bool data);
extern void platformDelayMs(uint32_t ms);

class NixieDisplay {
    public:
    NixieDisplay(uint8_t active, uint8_t offset, 
    uint8_t pinout1[], uint8_t pinout2[], uint8_t pinout3[], uint8_t pinout4[], uint8_t pinout5[], uint8_t pinout6[]);
    ~NixieDisplay(void);
    nixie_display_err_t init();
    nixie_display_err_t write(uint32_t num);
    nixie_display_err_t writeTime(struct tm *time);
    nixie_display_err_t writeSingleTube(uint8_t tube, uint8_t value);
    nixie_display_err_t clear();
    nixie_display_err_t setCrossfade(bool crossfade);
    nixie_display_err_t setScrollback(bool scrollback);
    nixie_display_err_t runProtection(nixie_display_protection_t type, uint32_t ms, uint32_t CATHODE_PROTECTION_INTER_MS = 15);
    private:
    nixie_display_err_t writeTubeInternal(uint8_t tube, uint8_t current, uint8_t prev);

};
#endif