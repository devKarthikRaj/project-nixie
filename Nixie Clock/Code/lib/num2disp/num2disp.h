/**
 @file num2disp.h
 @brief Library for interfacing with nixie tubes
 @author Edward62740
 */

#ifndef NUM2DISP_H
#define NUM2DISP_H

#include <Arduino.h>
#include <Wire.h>


#define CROSSFADE_PULSE_CYCLE_MS 20
#define CATHODE_PROTECTION_INTER_MS 15
#define CATHODE_PROTECTION_STYLE_WAVE 0
#define CATHODE_PROTECTION_STYLE_SLOT 1
typedef struct NumericalDisplayStruct
{
    uint8_t digit[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    uint8_t pinout[10];
    uint8_t index;
} NumericalDisplay_t;

struct FullDisplayStruct
{
    NumericalDisplay_t display[6]; // display index from right
    uint8_t num_active;
    uint8_t num_offset;
};

enum err_codes
{
    NO_ERR, ERR_FAIL, ERR_PARAM, ERR_INT
};

typedef int8_t num2disp_err_t;

extern bool num2disp_gpio_write(uint8_t pin, bool data);

num2disp_err_t num2disp_createInstanceNumericalDisplay(NumericalDisplay_t *num,
        uint8_t pinout[]);

num2disp_err_t num2disp_createInstanceFullDisplay(NumericalDisplay_t *num1,
        NumericalDisplay_t *num2, NumericalDisplay_t *num3,
        NumericalDisplay_t *num4, NumericalDisplay_t *num5,
        NumericalDisplay_t *num6, uint8_t num_active, uint8_t num_offset);

num2disp_err_t num2disp_writeNumberToFullDisplay(uint32_t num, uint32_t prev_num, bool crossfade);

num2disp_err_t num2disp_clearNumberFromFullDisplay();

num2disp_err_t num2disp_writeNumberToNumericalDisplay(uint8_t index, uint8_t digit, uint8_t prev_digit, bool crossfade);

num2disp_err_t num2disp_runCathodePoisoningProtection(uint32_t iterations, bool style);
#endif
