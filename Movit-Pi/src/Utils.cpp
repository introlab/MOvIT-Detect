#include "Utils.h"
#include <stdio.h>

uint8_t DECToBCD(const uint8_t &value)
{
    uint8_t ones = value % 10;
    uint8_t tens = (value / 10) << 4;

    // printf("DECToBCD value :%i\n", value);
    // printf("DECToBCD ones :%i\n", ones);
    // printf("DECToBCD tens :%i\n", tens);
    // printf("DECToBCD ret :%i\n", ones | tens);

    return (tens | ones);
}

uint8_t BCDToDEC(const uint8_t &value)
{
    // printf("BCDToDEC tens :%i\n", ((value & TENS_MASK) >> 4) * 10);
    // printf("BCDToDEC ones :%i\n", (value & ONES_MASK));
    return ((value & TENS_MASK) >> 4) * 10 + (value & ONES_MASK);
}

uint8_t BCDAdd(const uint8_t &BCDlvalue, const uint8_t &DECrvalue)
{
    uint8_t dec = BCDToDEC(BCDlvalue) + DECrvalue;

    // printf("BCDAdd dec :%i\n", dec);

    return DECToBCD(dec);
}

uint8_t BCDSubstract(const uint8_t &BCDlvalue, const uint8_t &DECrvalue)
{
    if (DECrvalue >= BCDToDEC(BCDlvalue))
    {
        return 0;
    }

    uint8_t dec = BCDToDEC(BCDlvalue) - DECrvalue;

    // printf("BCDSubstract dec :%i\n", dec);

    return DECToBCD(dec);
}