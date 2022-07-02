/**
 @file nixiedisplay.cpp
 @brief Library for interfacing with nixie tubes
 @author Edward62740
 */

#include "nixiedisplay.h"

struct DisplayStruct DisplayStruct_t;
uint8_t tmp_current[6];
uint8_t tmp_prev[6] = {9, 9, 9, 9, 9, 9};

/**
 @brief Constructor for NixieDisplay
 @param [in] active number of active tubes (max 6)
 @param [in] offset right shift
 @param [in] pinout1[] leftmost tube pinout array
 @param [in] pinout2[] 2nd from left pinout array
 @param [in] pinout3[] 3rd from left tube pinout array
 @param [in] pinout4[] 3rd from right tube pinout array
 @param [in] pinout5[] 2nd from right tube pinout array
 @param [in] pinout6[] rightmost tube pinout array
 @return NixieDisplay object
 */
NixieDisplay::NixieDisplay(uint8_t active, uint8_t offset,
                           uint8_t pinout1[], uint8_t pinout2[], uint8_t pinout3[], uint8_t pinout4[], uint8_t pinout5[], uint8_t pinout6[])
{
    DisplayStruct_t.active = active;
    DisplayStruct_t.offset = offset;

    for (int i = 0; i < 10; i++)
    {
        DisplayStruct_t.tube[0].pinout[i] = pinout1[i];
        DisplayStruct_t.tube[1].pinout[i] = pinout2[i];
        DisplayStruct_t.tube[2].pinout[i] = pinout3[i];
        DisplayStruct_t.tube[3].pinout[i] = pinout4[i];
        DisplayStruct_t.tube[4].pinout[i] = pinout5[i];
        DisplayStruct_t.tube[5].pinout[i] = pinout6[i];
    }
}

/**
 @brief Destructor for NixieDisplay
 */
NixieDisplay::~NixieDisplay(void)
{
    for (int j = 0; j < 6; j++)
    {
        memset(DisplayStruct_t.tube[j].pinout, 0, sizeof(DisplayStruct_t.tube[j].pinout));
    }
}

/**
 @brief Initializes the NixieDisplay
 @return NO_ERR if no error, else ERR_FAIL
 */
nixie_display_err_t NixieDisplay::init()
{
    nixie_display_err_t ret = NO_ERR;
    ret = clear();
    ret = write(999999);
    return ret;
}

/**
 @brief Writes a number to the NixieDisplay
 @param [in] num Number to be written to the NixieDisplay. Accepts a value from 0 to 999999, which is written left-justified
 @return NO_ERR if no error, ERR_PARAM for invalid value, ERR_INT for internal function error
 */
nixie_display_err_t NixieDisplay::write(uint32_t num)
{
    nixie_display_err_t ret = NO_ERR;
    if (num > 999999)
    {
        return ERR_PARAM;
    }
    for (int i = 0; i < 6; i++)
    {
        tmp_current[5 - i] = num % 10;
        num = (num - num % 10) / 10;
    }
    for (uint8_t n = 0; n < DisplayStruct_t.active; n++)
    {
        uint8_t current = tmp_current[n + DisplayStruct_t.offset];
        uint8_t prev = tmp_prev[n + DisplayStruct_t.offset];
        if (writeTubeInternal(n, current, prev) != NO_ERR)
        {
            return ERR_INT;
        }
    }
    memcpy(tmp_prev, tmp_current, sizeof(tmp_current));
    return ret;
}

/**
 @brief Writes a time to the NixieDisplay
 @param [in] *time Pointer to C tm structure
 @return NO_ERR if no error, ERR_PARAM for invalid value, ERR_INT for internal function error
 */
nixie_display_err_t NixieDisplay::writeTime(struct tm *time)
{
    nixie_display_err_t ret = NO_ERR;
    if (time == nullptr)
    {
        return ERR_PARAM;
    }
    tmp_current[5] = time->tm_sec % 10;
    if ((time->tm_sec - tmp_current[5]) == 0)
    {
        tmp_current[4] = 0;
    }
    else
    {
        tmp_current[4] = (time->tm_sec - tmp_current[5]) / 10;
    }
    tmp_current[3] = time->tm_min % 10;
    if ((time->tm_min - tmp_current[3]) == 0)
    {
        tmp_current[2] = 0;
    }
    else
    {
        tmp_current[2] = (time->tm_min - tmp_current[3]) / 10;
    }
    tmp_current[1] = time->tm_hour % 10;
    if ((time->tm_hour - tmp_current[1]) == 0)
    {
        tmp_current[0] = 0;
    }
    else
    {
        tmp_current[0] = (time->tm_hour - tmp_current[1]) / 10;
    }
    for (uint8_t n = 0; n < DisplayStruct_t.active; n++)
    {
        uint8_t current = tmp_current[n + DisplayStruct_t.offset];
        uint8_t prev = tmp_prev[n + DisplayStruct_t.offset];
        if (current != prev)
        {
            if (writeTubeInternal(n, current, prev) != NO_ERR)
            {
                return ERR_INT;
            }
        }
    }
    memcpy(tmp_prev, tmp_current, sizeof(tmp_current));
    return ret;
}

/**
 @brief Writes to a single tube
 @param [in] tube Index from left of tube to be written
 @param [in] value Integer to be written from 0-9
 @return NO_ERR if no error, ERR_PARAM for invalid value, ERR_INT for internal function error
 */
nixie_display_err_t NixieDisplay::writeSingleTube(uint8_t tube, uint8_t value)
{
    nixie_display_err_t ret = NO_ERR;
    if (value > 9 || value < 0)
    {
        return ERR_PARAM;
    }
    tmp_current[tube - 1] = (uint8_t)value;
    if (writeTubeInternal(tube - 1, tmp_current[tube - 1 + DisplayStruct_t.offset], tmp_prev[tube - 1 + DisplayStruct_t.offset]) != NO_ERR)
    {
        return ERR_INT;
    }
    memcpy(tmp_prev, tmp_current, sizeof(tmp_current));
    return ret;
}

/**
 @brief Internal function to write to tube and execute crossfade and scrollback effects
 @param [in] tube Index from left of tube to be written
 @param [in] current Integer to be displayed on tube
 @param [in] prev Current integer on tube
 @return NO_ERR if no error, ERR_FAIL for failed write, ERR_INT for internal function error
 */
nixie_display_err_t NixieDisplay::writeTubeInternal(uint8_t tube, uint8_t current, uint8_t prev)
{
    nixie_display_err_t ret = NO_ERR;
    int8_t gpio_digit_current =
        (uint8_t)DisplayStruct_t.tube[tube].pinout[current];
    uint8_t gpio_digit_prev =
        (uint8_t)DisplayStruct_t.tube[tube].pinout[prev];
    if (DisplayStruct_t.scrollback && current == 0 && prev != 0)
    {
        if (!platformGPIOWrite(DisplayStruct_t.tube[tube].pinout[prev], 0))
        {
            return ERR_FAIL;
        }
        for (uint8_t i = prev; i > 0; i--)
        {
            if (!platformGPIOWrite(DisplayStruct_t.tube[tube].pinout[i], 1))
            {
                return ERR_FAIL;
            }
            platformDelayMs(CROSSFADE_PULSE_CYCLE_MS);
            if (!platformGPIOWrite(DisplayStruct_t.tube[tube].pinout[i], 0))
            {
                return ERR_FAIL;
            }
        }
        if (!platformGPIOWrite(gpio_digit_current, 1))
        {
            return ERR_FAIL;
        }
    }
    else if (DisplayStruct_t.crossfade && (gpio_digit_current != gpio_digit_prev))
    {
        for (float i = 0.2; i <= 0.8; i += 0.1)
        {
            if (!platformGPIOWrite(gpio_digit_current, 0))
            {
                return ERR_FAIL;
            }
            if (!platformGPIOWrite(gpio_digit_prev, 1))
            {
                return ERR_FAIL;
            }
            platformDelayMs(CROSSFADE_PULSE_CYCLE_MS * (1 - i));
            if (!platformGPIOWrite(gpio_digit_current, 1))
            {
                return ERR_FAIL;
            }
            if (!platformGPIOWrite(gpio_digit_prev, 0))
            {
                return ERR_FAIL;
            }
            platformDelayMs(CROSSFADE_PULSE_CYCLE_MS * i);
        }
    }
    else if (!DisplayStruct_t.crossfade && (gpio_digit_current != gpio_digit_prev))
    {
        if (!platformGPIOWrite(gpio_digit_current, 1))
        {
            return ERR_FAIL;
        }
        if (!platformGPIOWrite(gpio_digit_prev, 0))
        {
            return ERR_FAIL;
        }
    }
    else
    {
        if (!platformGPIOWrite(gpio_digit_current, 1))
        {
            return ERR_FAIL;
        }
    }
    return ret;
}

/**
 @brief Clears all numbers from the NixieDisplay
 @return NO_ERR if no error, ERR_FAIL for failed write
 */
nixie_display_err_t NixieDisplay::clear()
{
    nixie_display_err_t ret = NO_ERR;
    for (uint8_t i = 0; i < DisplayStruct_t.active; i++)
    {
        for (uint8_t j = 0; j < 10; j++)
        {
            if (platformGPIOWrite(
                    (uint8_t)(!DisplayStruct_t.tube[i + DisplayStruct_t.offset].pinout[j]), 0))
            {
                return ERR_FAIL;
            }
        }
    }
    return ret;
}

/**
 @brief Option to select crossfade effect
 @param [in] crossfade
 @return NO_ERR if no error
 */
nixie_display_err_t NixieDisplay::setCrossfade(bool crossfade)
{
    nixie_display_err_t ret = NO_ERR;
    DisplayStruct_t.crossfade = (bool)crossfade;
    return ret;
}

/**
 @brief Option to select scrollback effect
 @param [in] crossfade
 @return NO_ERR if no error
 */
nixie_display_err_t NixieDisplay::setScrollback(bool scrollback)
{
    nixie_display_err_t ret = NO_ERR;
    DisplayStruct_t.scrollback = (bool)scrollback;
    return ret;
}

/**
 @brief Runs cathode protection
 @param [in] type Protection visual effect
 @param [in] ms Time to run in ms
 @param [in] CATHODE_PROTECTION_INTER_MS Time spent on each digit in ms, default is 15
 @return NO_ERR if no error, ERR_PARAM if invalid parameters, ERR_INT for internal function error
 */
nixie_display_err_t NixieDisplay::runProtection(nixie_display_protection_t type, uint32_t ms, uint32_t CATHODE_PROTECTION_INTER_MS)
{
    nixie_display_err_t ret = NO_ERR;
    uint32_t iterations = 0;
    uint32_t tmp_internal = 0;
    uint8_t tmp_num[6];
    memcpy(tmp_num, tmp_current, sizeof(tmp_current));
    bool tmp_crossfade = DisplayStruct_t.crossfade;
    bool tmp_scrollback = DisplayStruct_t.scrollback;
    if (setCrossfade(false) != NO_ERR)
    {
        return ERR_INT;
    }
    if (setScrollback(false) != NO_ERR)
    {
        return ERR_INT;
    }
    if (ms < CATHODE_PROTECTION_INTER_MS * 10)
    {
        return ERR_PARAM;
    }
    else
    {
        iterations = ms / (CATHODE_PROTECTION_INTER_MS * 10);
    }

    for (int n = 0; n < iterations; n++)
    {
        if (type == CATHODE_PROTECTION_STYLE_SLOT)
        {
            for (int i = 0; i < 10; i++)
            {
                if (tmp_internal == 0)
                {
                    ret = write(0);
                }
                else
                {
                    ret = write(tmp_internal);
                    if (ret != NO_ERR)
                    {
                        return ret;
                    }
                }
                if (tmp_internal == 999999)
                {
                    tmp_internal = 0;
                }
                else
                {
                    tmp_internal += 111111;
                }
                platformDelayMs(CATHODE_PROTECTION_INTER_MS);
            }
        }
        else if (type == CATHODE_PROTECTION_STYLE_WAVE)
        {
            ret = write(123456);
            platformDelayMs(CATHODE_PROTECTION_INTER_MS);
            ret = write(234567);
            platformDelayMs(CATHODE_PROTECTION_INTER_MS);
            ret = write(345678);
            platformDelayMs(CATHODE_PROTECTION_INTER_MS);
            ret = write(456789);
            platformDelayMs(CATHODE_PROTECTION_INTER_MS);
            ret = write(567890);
            platformDelayMs(CATHODE_PROTECTION_INTER_MS);
            ret = write(678901);
            platformDelayMs(CATHODE_PROTECTION_INTER_MS);
            ret = write(789012);
            platformDelayMs(CATHODE_PROTECTION_INTER_MS);
            ret = write(890123);
            platformDelayMs(CATHODE_PROTECTION_INTER_MS);
            ret = write(901234);
            platformDelayMs(CATHODE_PROTECTION_INTER_MS);
            ret = write(012345);
            platformDelayMs(CATHODE_PROTECTION_INTER_MS);
        }
        else if (type == CATHODE_PROTECTION_STYLE_SEQUENTIAL)
        {
            ret = write(111111);
            platformDelayMs(CATHODE_PROTECTION_INTER_MS);
            ret = write(000000);
            platformDelayMs(CATHODE_PROTECTION_INTER_MS);
            ret = write(222222);
            platformDelayMs(CATHODE_PROTECTION_INTER_MS);
            ret = write(999999);
            platformDelayMs(CATHODE_PROTECTION_INTER_MS);
            ret = write(333333);
            platformDelayMs(CATHODE_PROTECTION_INTER_MS);
            ret = write(888888);
            platformDelayMs(CATHODE_PROTECTION_INTER_MS);
            ret = write(444444);
            platformDelayMs(CATHODE_PROTECTION_INTER_MS);
            ret = write(777777);
            platformDelayMs(CATHODE_PROTECTION_INTER_MS);
            ret = write(555555);
            platformDelayMs(CATHODE_PROTECTION_INTER_MS);
            ret = write(666666);
            platformDelayMs(CATHODE_PROTECTION_INTER_MS);
        }
    }

    setCrossfade(tmp_crossfade);
    setScrollback(tmp_scrollback);
    for (uint8_t n = 0; n < DisplayStruct_t.active; n++)
    {
        uint8_t current = tmp_current[n + DisplayStruct_t.offset];
        uint8_t prev = tmp_prev[n + DisplayStruct_t.offset];
        if (writeTubeInternal(n, current, prev) != NO_ERR)
        {
            return ERR_INT;
        }
    }

    return ret;
}
