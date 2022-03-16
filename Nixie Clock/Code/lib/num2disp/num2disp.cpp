/**
 @file num2disp.cpp
 @brief Library for interfacing with nixie tubes
 @author Edward62740
 */

#include "num2disp.h"


struct FullDisplayStruct FullDisplay;
uint8_t internal_num_current[6];
uint8_t internal_num_prev[6];


/**
 @brief map digits to pinout for a single display
 @param [in] *num pointer to NumericalDisplay_t
 @param [in] pinout[]
 @return NO_ERR on success
 */
num2disp_err_t num2disp_createInstanceNumericalDisplay(NumericalDisplay_t *num,
        uint8_t pinout[])
{
    for (int i = 0; i <= 10; i++)
    {
        num->pinout[i] = pinout[i];
    }
    return NO_ERR;
}

/**
 @brief combine single displays into a group
 @param [in] *num1:6 pointer to NumericalDisplay_t
 @param [in] num_active number of active digits from left (max 6)
 @param [in] num_offset right shift digits
 @return NO_ERR on success
 */
num2disp_err_t num2disp_createInstanceFullDisplay(NumericalDisplay_t *num1,
        NumericalDisplay_t *num2, NumericalDisplay_t *num3,
        NumericalDisplay_t *num4, NumericalDisplay_t *num5,
        NumericalDisplay_t *num6, uint8_t num_active, uint8_t num_offset)
{
    FullDisplay.display[0] = *num1;
    FullDisplay.display[1] = *num2;
    FullDisplay.display[2] = *num3;
    FullDisplay.display[3] = *num4;
    FullDisplay.display[4] = *num5;
    FullDisplay.display[5] = *num6;
    FullDisplay.num_active = num_active;
    FullDisplay.num_offset = num_offset;
    return NO_ERR;
}

/**
 @brief write a maximum of 6 digits to the displays
 @param [in] num number to be written
 @param [in] prev_num previous number on the display
 @param [in] crossfade option to crossfade write
 @return NO_ERR on success
 */
num2disp_err_t num2disp_writeNumberToFullDisplay(uint32_t num,
        uint32_t prev_num, bool crossfade)
{

    num2disp_err_t ret = 0;

    if (num > 999999 || prev_num > 999999)
    {
        return ERR_PARAM;
    }
    for (int i = 0; i < 6; i++)
    {

        internal_num_current[5 - i] = num % 10;
        num = (num - num % 10) / 10;
    }
    for (int i = 0; i < 6; i++)
    {

        internal_num_prev[5 - i] = prev_num % 10;
        prev_num = (prev_num - prev_num % 10) / 10;
    }
    for (uint8_t n = 0; n < FullDisplay.num_active; n++)
    {
        uint8_t current = internal_num_current[n + FullDisplay.num_offset];
        uint8_t prev = internal_num_prev[n + FullDisplay.num_offset];
        ret = num2disp_writeNumberToNumericalDisplay(n + FullDisplay.num_offset,
                current, prev, crossfade);
    }

    return ret;

}

/**
 @brief clear all states of display
 @return NO_ERR on success
 */
num2disp_err_t num2disp_clearNumberFromFullDisplay()
{
    num2disp_err_t ret = 0;
    for (uint8_t i = 0; i < FullDisplay.num_active; i++)
    {
        for (uint8_t j = 0; j < 10; j++)
        {
            ret = num2disp_gpio_write(
                      (uint8_t)(
                          FullDisplay.display[i + FullDisplay.num_offset].pinout[j]),
                      0);
        }
    }

    return ret;
}

/**
 @brief write a digit to a single display
 @param [in] index order of display from left
 @param [in] digit digit to be written
 @param [in] prev_digit previous digit
 @param [in] crossfade option to crossfade write
 @return NO_ERR on success, gpio write error count otherwise
 */
num2disp_err_t num2disp_writeNumberToNumericalDisplay(uint8_t index,
        uint8_t digit, uint8_t prev_digit, bool crossfade)
{
    num2disp_err_t ret = 0;
    uint8_t gpio_digit_current =
        (uint8_t) FullDisplay.display[index].pinout[digit]; //get pin number from NumericalDisplayStruct
    uint8_t gpio_digit_prev =
        (uint8_t) FullDisplay.display[index].pinout[prev_digit];

    if (crossfade && (gpio_digit_current != gpio_digit_prev))
    {

        for (float i = 0.2; i<=0.8 ; i+=0.1)
        {
            ret += num2disp_gpio_write(gpio_digit_current, 0);
            ret += num2disp_gpio_write(gpio_digit_prev, 1);
            delay(CROSSFADE_PULSE_CYCLE_MS * (1-i));
            ret += num2disp_gpio_write(gpio_digit_current, 1);
            ret += num2disp_gpio_write(gpio_digit_prev, 0);
            delay(CROSSFADE_PULSE_CYCLE_MS * i);
        }

    }
    else if (!crossfade && (gpio_digit_current != gpio_digit_prev))
    {
        ret += num2disp_gpio_write(gpio_digit_current, 1);
        ret += num2disp_gpio_write(gpio_digit_prev, 0);
    }
    else
    {
        ret += num2disp_gpio_write(gpio_digit_current, 1);
    }

    return ret;
}


/**
 @brief run cathode protection routine
 @param [in] iterations number of times to run through digits 0-9
 @param [in] style visual style
 @return NO_ERR on success
 */
num2disp_err_t num2disp_runCathodePoisoningProtection(uint32_t iterations,
        bool style)
{
    num2disp_err_t ret = 0;
    uint32_t num = 0;
    for (uint32_t i = 0; i < 10 * iterations; i++)
    {

        if (style)
        {

            if (num == 0)
            {
                ret = num2disp_writeNumberToFullDisplay(0, 999999, false);
            }

            else
            {
                ret = num2disp_writeNumberToFullDisplay(num, num - 111111, false);
            }

            if (num == 999999)
            {
                num = 0;
            }
            else
            {
                num += 111111;
            }
        }
        else
        {
            if (num == 0)
            {
                num = 123456;
            }

            if (num >= 123456 && num <= 345678)
            {
                ret = num2disp_writeNumberToFullDisplay(num, num - 111111, false);
                num += 111111;
            }
            else if (num == 456789)
            {
                ret = num2disp_writeNumberToFullDisplay(num, num - 111111, false);
                num += 111101;
            }
            else if (num == 567890)
            {
                ret = num2disp_writeNumberToFullDisplay(num, 456789, false);
                num += 111011;
            }
            else if (num == 678901)
            {
                ret = num2disp_writeNumberToFullDisplay(num, 567890, false);
                num += 110111;
            }
            else if (num == 789012)
            {
                ret = num2disp_writeNumberToFullDisplay(num, 678901, false);
                num += 101111;
            }
            else if (num == 890123)
            {
                ret = num2disp_writeNumberToFullDisplay(num, 789012, false);
                num += 11111;
            }
            else if (num == 901234)
            {
                ret = num2disp_writeNumberToFullDisplay(num, 890123, false);
                num -= 888889;
            }
            else if (num == 12345)
            {
                ret = num2disp_writeNumberToFullDisplay(num, 901234, false);
                num += 111111;
            }

        }
        delay(CATHODE_PROTECTION_INTER_MS);

    }
    ret = num2disp_clearNumberFromFullDisplay();

    return ret;

}
