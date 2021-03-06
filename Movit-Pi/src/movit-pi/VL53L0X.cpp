//
// VL53L0X time of flight range sensor
// Library to read the distance
// from the I2C bus
//
// by Larry Bank
//
// This code is based on Pololu's Arduino library
// https://github.com/pololu/vl53l0x-arduino
// (see LICENSE.txt for more info)
//
// My version is an attempt to simplify that code and
// create a generic C library for Linux
//

#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include "I2Cdev.h"
#include "VL53L0X.h"
#include "tof.h"

VL53L0X::VL53L0X()
    : isInitialized(false), address(ADDRESS_DEFAULT)
{

}


int VL53L0X::ReadRangeSingleMillimeters()
{
    return tofReadDistance();
}

bool VL53L0X::Initialize()
{
    isInitialized = tofInit(1, address, 1);
    printf("Vl53L0X::Initialize : %i\n", isInitialized);
    return isInitialized;
}

bool VL53L0X::IsConnected() {
    int model = 0;
    int revision = 0;
    if (tofGetModel(&model, &revision))
    {
        //printf("model: %i, revision : %i\n", model, revision);
        if (model == 238 && revision  == 16)
            return true;
    }
    return false;
}


#if 0

// Defines /////////////////////////////////////////////////////////////////////
#define calcMacroPeriod(vcsel_period_pclks) ((((uint32_t)2304 * (vcsel_period_pclks) * 1655) + 500) / 1000)
// Encode VCSEL pulse period register value from period in PCLKs
// based on VL53L0X_encode_vcsel_period()
#define encodeVcselPeriod(period_pclks) (((period_pclks) >> 1) - 1)

//
// Register init lists consist of the count followed by register/value pairs
//
namespace {

unsigned char ucI2CMode[] = {4, 0x88,0x00, 0x80,0x01, 0xff,0x01, 0x00,0x00};
unsigned char ucI2CMode2[] = {3, 0x00,0x01, 0xff,0x00, 0x80,0x00};
unsigned char ucSPAD0[] = {4, 0x80,0x01, 0xff,0x01, 0x00,0x00, 0xff,0x06};
unsigned char ucSPAD1[] = {5, 0xff,0x07, 0x81,0x01, 0x80,0x01, 0x94,0x6b, 0x83,0x00};
unsigned char ucSPAD2[] = {4, 0xff,0x01, 0x00,0x01, 0xff,0x00, 0x80,0x00};
unsigned char ucSPAD[] = {5, 0xff,0x01, 0x4f,0x00, 0x4e,0x2c, 0xff,0x00, 0xb6,0xb4};
unsigned char ucDefTuning[] = {80, 0xff,0x01, 0x00,0x00, 0xff,0x00, 0x09,0x00,
                               0x10,0x00, 0x11,0x00, 0x24,0x01, 0x25,0xff, 0x75,0x00, 0xff,0x01, 0x4e,0x2c,
                               0x48,0x00, 0x30,0x20, 0xff,0x00, 0x30,0x09, 0x54,0x00, 0x31,0x04, 0x32,0x03,
                               0x40,0x83, 0x46,0x25, 0x60,0x00, 0x27,0x00, 0x50,0x06, 0x51,0x00, 0x52,0x96,
                               0x56,0x08, 0x57,0x30, 0x61,0x00, 0x62,0x00, 0x64,0x00, 0x65,0x00, 0x66,0xa0,
                               0xff,0x01, 0x22,0x32, 0x47,0x14, 0x49,0xff, 0x4a,0x00, 0xff,0x00, 0x7a,0x0a,
                               0x7b,0x00, 0x78,0x21, 0xff,0x01, 0x23,0x34, 0x42,0x00, 0x44,0xff, 0x45,0x26,
                               0x46,0x05, 0x40,0x40, 0x0e,0x06, 0x20,0x1a, 0x43,0x40, 0xff,0x00, 0x34,0x03,
                               0x35,0x44, 0xff,0x01, 0x31,0x04, 0x4b,0x09, 0x4c,0x05, 0x4d,0x04, 0xff,0x00,
                               0x44,0x00, 0x45,0x20, 0x47,0x08, 0x48,0x28, 0x67,0x00, 0x70,0x04, 0x71,0x01,
                               0x72,0xfe, 0x76,0x00, 0x77,0x00, 0xff,0x01, 0x0d,0x01, 0xff,0x00, 0x80,0x01,
                               0x01,0xf8, 0xff,0x01, 0x8e,0x01, 0x00,0x01, 0xff,0x00, 0x80,0x00};
}

VL53L0X::VL53L0X()
    : isInitialized(false), address(ADDRESS_DEFAULT)
{

}


bool VL53L0X::Initialize()
{
    // VL53L0X_DataInit() begin
    // sensor uses 1V8 mode for I/O by default; switch to 2V8 mode if necessary
    printf("VL53L0X::Initialize()\n");



    if(!IsConnectedOnly()) {
        isInitialized = false;
        return false;
    }


    isInitialized = initSensor(1);
    printf("initSensor initialized: %i\n",isInitialized);


    return isInitialized;
}


unsigned short VL53L0X::readReg16(unsigned char ucAddr)
{
    uint16_t data;
    if (I2Cdev::ReadWord(address, ucAddr, &data))
    {

        return data;
    }

    printf("VL53L0X::readReg16 - Error\n");
    return 0;
}

unsigned char VL53L0X::readReg(unsigned char ucAddr)
{
    uint8_t data;
    if (I2Cdev::ReadByte(address, ucAddr, &data))
    {
        return data;
    }

    printf(" VL53L0X::readReg - Error\n");
    return 0;
}

void VL53L0X::readMulti(unsigned char ucAddr, unsigned char *pBuf, int iCount)
{
    if (!I2Cdev::ReadBytes(address, ucAddr, iCount, pBuf))
    {
        printf("Error readMulti\n");
    }
}

void VL53L0X::writeMulti(unsigned char ucAddr, unsigned char *pBuf, int iCount)
{
    if (!I2Cdev::WriteBytes(address, ucAddr, iCount, pBuf))
    {
        printf("Error writeMulti\n");
    }

    /*
    for (auto i = 0; i < iCount; i++)
    {
        uint8_t testValue = 0;
        I2Cdev::ReadByte(address,ucAddr, &testValue);
        if ( testValue != pBuf[i])
        {
            printf("writeMulti mismatch :%i\n",i);
        }
    }
    */
}

void VL53L0X::writeReg16(unsigned char ucAddr, unsigned short usValue)
{
    if (!I2Cdev::WriteWord(address, ucAddr, usValue))
    {
        printf("Error writeReg16\n");
    }
    /*
    unsigned short testValue = 0;
    I2Cdev::ReadWord(address,ucAddr, &testValue);
    if ( testValue != usValue)
    {
        printf("writeReg16 mismatch \n");
    }
    */


}

void VL53L0X::writeReg(unsigned char ucAddr, unsigned char ucValue)
{
    if (!I2Cdev::WriteByte(address, ucAddr, ucValue))
    {
        printf("Error writeReg\n");
    }
    /*
    unsigned char testValue = 0;
    I2Cdev::ReadByte(address,ucAddr, &testValue);
    if ( testValue != ucValue)
    {
        printf("writeReg mismatch reg: %2.2x, val1: %2.2x val2: %2.2x\n",ucAddr, testValue, ucValue);
    }
    */
}

void VL53L0X::writeRegList(unsigned char *ucList)
{
    unsigned char ucCount = *ucList++; // count is the first element in the list

    for (auto i =0; i < ucCount; i++)
    {
        unsigned char reg = *ucList++;
        unsigned char val = *ucList++;
        //printf("WriteRegList : %2.2x, %2.2x\n", reg, val);
        writeReg(reg, val);
    }
}



bool VL53L0X::IsConnectedOnly() {


    int model = 0;
    int revision = 0;

    tofGetModel(&model, &revision);


    if (model == 238 && revision == 16)
    {
        return true;
    }
    else {
       printf("VL53L0X WRONG model %i, revision %i \n", model, revision);
       return false;
    }
    /*
    bool connected = true;
    uint8_t data;
    if(I2Cdev::ReadByte(address, 0xC0, &data)) {
        connected = connected && (data == 0xee);
    } else {
        //We try a reset...
        I2Cdev::WriteByte(address, 0xBF, 0x01);
        usleep(100000);                        //100 ms
        return false;
    }
    if(I2Cdev::ReadByte(address, 0xC1, &data)) {
        connected = connected && (data == 0xaa);
    } else {
        //We try a reset...
        I2Cdev::WriteByte(address, 0xBF, 0x01);
        usleep(100000);                        //100 ms
        return false;
    }
    if(I2Cdev::ReadByte(address, 0xC2, &data)) {
        connected = connected && (data == 0x10);
    } else {
        //We try a reset...
        I2Cdev::WriteByte(address, 0xBF, 0x01);
        usleep(100000);                        //100 ms
        return false;
    }
    return connected;
    */

}

bool VL53L0X::IsConnected() {
    bool connected = IsConnectedOnly();
    
    if(connected == false){
        isInitialized = false;
    }
    return connected && isInitialized;
}

int VL53L0X::ReadRangeSingleMillimeters()
{

    int val = tofReadDistance();
    if( val < 4096)
        return val;


    return -1;
}

int VL53L0X::getSpadInfo(unsigned char *pCount, unsigned char *pTypeIsAperture)
{
    int iTimeout;
    unsigned char ucTemp;
#define MAX_TIMEOUT 50

    writeRegList(ucSPAD0);
    writeReg(0x83, readReg(0x83) | 0x04);
    writeRegList(ucSPAD1);
    iTimeout = 0;
    while(iTimeout < MAX_TIMEOUT)
    {
        if (readReg(0x83) != 0x00) break;
        iTimeout++;
        usleep(5000);
    }
    if (iTimeout == MAX_TIMEOUT)
    {
        fprintf(stderr, "Timeout while waiting for SPAD info\n");
        return 0;
    }
    writeReg(0x83,0x01);
    ucTemp = readReg(0x92);
    *pCount = (ucTemp & 0x7f);
    *pTypeIsAperture = (ucTemp & 0x80);
    writeReg(0x81,0x00);
    writeReg(0xff,0x06);
    writeReg(0x83, readReg(0x83) & ~0x04);
    writeRegList(ucSPAD2);

    return 1;
} /* getSpadInfo() */

// Decode sequence step timeout in MCLKs from register value
// based on VL53L0X_decode_timeout()
// Note: the original function returned a uint32_t, but the return value is
// always stored in a uint16_t.
uint16_t VL53L0X::decodeTimeout(uint16_t reg_val)
{
    // format: "(LSByte * 2^MSByte) + 1"
    return (uint16_t)((reg_val & 0x00FF) <<
                      (uint16_t)((reg_val & 0xFF00) >> 8)) + 1;
}

// Convert sequence step timeout from MCLKs to microseconds with given VCSEL period in PCLKs
// based on VL53L0X_calc_timeout_us()
uint32_t VL53L0X::timeoutMclksToMicroseconds(uint16_t timeout_period_mclks, uint8_t vcsel_period_pclks)
{
    uint32_t macro_period_ns = calcMacroPeriod(vcsel_period_pclks);

    return ((timeout_period_mclks * macro_period_ns) + (macro_period_ns / 2)) / 1000;
}

// Convert sequence step timeout from microseconds to MCLKs with given VCSEL period in PCLKs
// based on VL53L0X_calc_timeout_mclks()
uint32_t VL53L0X::timeoutMicrosecondsToMclks(uint32_t timeout_period_us, uint8_t vcsel_period_pclks)
{
    uint32_t macro_period_ns = calcMacroPeriod(vcsel_period_pclks);

    return (((timeout_period_us * 1000) + (macro_period_ns / 2)) / macro_period_ns);
}

// Encode sequence step timeout register value from timeout in MCLKs
// based on VL53L0X_encode_timeout()
// Note: the original function took a uint16_t, but the argument passed to it
// is always a uint16_t.
uint16_t VL53L0X::encodeTimeout(uint16_t timeout_mclks)
{
    // format: "(LSByte * 2^MSByte) + 1"

    uint32_t ls_byte = 0;
    uint16_t ms_byte = 0;

    if (timeout_mclks > 0)
    {
        ls_byte = timeout_mclks - 1;

        while ((ls_byte & 0xFFFFFF00) > 0)
        {
            ls_byte >>= 1;
            ms_byte++;
        }

        return (ms_byte << 8) | (ls_byte & 0xFF);
    }
    else { return 0; }
}

void VL53L0X::getSequenceStepTimeouts(uint8_t enables, SequenceStepTimeouts * timeouts)
{
    timeouts->pre_range_vcsel_period_pclks = ((readReg(PRE_RANGE_CONFIG_VCSEL_PERIOD) +1) << 1);

    timeouts->msrc_dss_tcc_mclks = readReg(MSRC_CONFIG_TIMEOUT_MACROP) + 1;
    timeouts->msrc_dss_tcc_us =
            timeoutMclksToMicroseconds(timeouts->msrc_dss_tcc_mclks,
                                       timeouts->pre_range_vcsel_period_pclks);

    timeouts->pre_range_mclks =
            decodeTimeout(readReg16(PRE_RANGE_CONFIG_TIMEOUT_MACROP_HI));
    timeouts->pre_range_us =
            timeoutMclksToMicroseconds(timeouts->pre_range_mclks,
                                       timeouts->pre_range_vcsel_period_pclks);

    timeouts->final_range_vcsel_period_pclks = ((readReg(FINAL_RANGE_CONFIG_VCSEL_PERIOD) +1) << 1);

    timeouts->final_range_mclks =
            decodeTimeout(readReg16(FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI));

    if (enables & SEQUENCE_ENABLE_PRE_RANGE)
    {
        timeouts->final_range_mclks -= timeouts->pre_range_mclks;
    }

    timeouts->final_range_us =
            timeoutMclksToMicroseconds(timeouts->final_range_mclks,
                                       timeouts->final_range_vcsel_period_pclks);
} /* getSequenceStepTimeouts() */


// Set the VCSEL (vertical cavity surface emitting laser) pulse period for the
// given period type (pre-range or final range) to the given value in PCLKs.
// Longer periods seem to increase the potential range of the sensor.
// Valid values are (even numbers only):
//  pre:  12 to 18 (initialized default: 14)
//  final: 8 to 14 (initialized default: 10)
// based on VL53L0X_set_vcsel_pulse_period()
int VL53L0X::setVcselPulsePeriod(vcselPeriodType type, uint8_t period_pclks)
{
    uint8_t vcsel_period_reg = encodeVcselPeriod(period_pclks);

    uint8_t enables;
    SequenceStepTimeouts timeouts;

    enables = readReg(SYSTEM_SEQUENCE_CONFIG);
    getSequenceStepTimeouts(enables, &timeouts);

    // "Apply specific settings for the requested clock period"
    // "Re-calculate and apply timeouts, in macro periods"

    // "When the VCSEL period for the pre or final range is changed,
    // the corresponding timeout must be read from the device using
    // the current VCSEL period, then the new VCSEL period can be
    // applied. The timeout then must be written back to the device
    // using the new VCSEL period.
    //
    // For the MSRC timeout, the same applies - this timeout being
    // dependant on the pre-range vcsel period."


    if (type == VcselPeriodPreRange)
    {
        // "Set phase check limits"
        switch (period_pclks)
        {
        case 12:
            writeReg(PRE_RANGE_CONFIG_VALID_PHASE_HIGH, 0x18);
            break;

        case 14:
            writeReg(PRE_RANGE_CONFIG_VALID_PHASE_HIGH, 0x30);
            break;

        case 16:
            writeReg(PRE_RANGE_CONFIG_VALID_PHASE_HIGH, 0x40);
            break;

        case 18:
            writeReg(PRE_RANGE_CONFIG_VALID_PHASE_HIGH, 0x50);
            break;

        default:
            // invalid period
            return 0;
        }
        writeReg(PRE_RANGE_CONFIG_VALID_PHASE_LOW, 0x08);

        // apply new VCSEL period
        writeReg(PRE_RANGE_CONFIG_VCSEL_PERIOD, vcsel_period_reg);

        // update timeouts

        // set_sequence_step_timeout() begin
        // (SequenceStepId == VL53L0X_SEQUENCESTEP_PRE_RANGE)

        uint16_t new_pre_range_timeout_mclks =
                timeoutMicrosecondsToMclks(timeouts.pre_range_us, period_pclks);

        writeReg16(PRE_RANGE_CONFIG_TIMEOUT_MACROP_HI,
                   encodeTimeout(new_pre_range_timeout_mclks));

        // set_sequence_step_timeout() end

        // set_sequence_step_timeout() begin
        // (SequenceStepId == VL53L0X_SEQUENCESTEP_MSRC)

        uint16_t new_msrc_timeout_mclks =
                timeoutMicrosecondsToMclks(timeouts.msrc_dss_tcc_us, period_pclks);

        writeReg(MSRC_CONFIG_TIMEOUT_MACROP,
                 (new_msrc_timeout_mclks > 256) ? 255 : (new_msrc_timeout_mclks - 1));

        // set_sequence_step_timeout() end
    }
    else if (type == VcselPeriodFinalRange)
    {
        switch (period_pclks)
        {
        case 8:
            writeReg(FINAL_RANGE_CONFIG_VALID_PHASE_HIGH, 0x10);
            writeReg(FINAL_RANGE_CONFIG_VALID_PHASE_LOW,  0x08);
            writeReg(GLOBAL_CONFIG_VCSEL_WIDTH, 0x02);
            writeReg(ALGO_PHASECAL_CONFIG_TIMEOUT, 0x0C);
            writeReg(0xFF, 0x01);
            writeReg(ALGO_PHASECAL_LIM, 0x30);
            writeReg(0xFF, 0x00);
            break;

        case 10:
            writeReg(FINAL_RANGE_CONFIG_VALID_PHASE_HIGH, 0x28);
            writeReg(FINAL_RANGE_CONFIG_VALID_PHASE_LOW,  0x08);
            writeReg(GLOBAL_CONFIG_VCSEL_WIDTH, 0x03);
            writeReg(ALGO_PHASECAL_CONFIG_TIMEOUT, 0x09);
            writeReg(0xFF, 0x01);
            writeReg(ALGO_PHASECAL_LIM, 0x20);
            writeReg(0xFF, 0x00);
            break;

        case 12:
            writeReg(FINAL_RANGE_CONFIG_VALID_PHASE_HIGH, 0x38);
            writeReg(FINAL_RANGE_CONFIG_VALID_PHASE_LOW,  0x08);
            writeReg(GLOBAL_CONFIG_VCSEL_WIDTH, 0x03);
            writeReg(ALGO_PHASECAL_CONFIG_TIMEOUT, 0x08);
            writeReg(0xFF, 0x01);
            writeReg(ALGO_PHASECAL_LIM, 0x20);
            writeReg(0xFF, 0x00);
            break;

        case 14:
            writeReg(FINAL_RANGE_CONFIG_VALID_PHASE_HIGH, 0x48);
            writeReg(FINAL_RANGE_CONFIG_VALID_PHASE_LOW,  0x08);
            writeReg(GLOBAL_CONFIG_VCSEL_WIDTH, 0x03);
            writeReg(ALGO_PHASECAL_CONFIG_TIMEOUT, 0x07);
            writeReg(0xFF, 0x01);
            writeReg(ALGO_PHASECAL_LIM, 0x20);
            writeReg(0xFF, 0x00);
            break;

        default:
            // invalid period
            return 0;
        }

        // apply new VCSEL period
        writeReg(FINAL_RANGE_CONFIG_VCSEL_PERIOD, vcsel_period_reg);

        // update timeouts

        // set_sequence_step_timeout() begin
        // (SequenceStepId == VL53L0X_SEQUENCESTEP_FINAL_RANGE)

        // "For the final range timeout, the pre-range timeout
        //  must be added. To do this both final and pre-range
        //  timeouts must be expressed in macro periods MClks
        //  because they have different vcsel periods."

        uint16_t new_final_range_timeout_mclks =
                timeoutMicrosecondsToMclks(timeouts.final_range_us, period_pclks);

        if (enables & SEQUENCE_ENABLE_PRE_RANGE)
        {
            new_final_range_timeout_mclks += timeouts.pre_range_mclks;
        }

        writeReg16(FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI,
                   encodeTimeout(new_final_range_timeout_mclks));

        // set_sequence_step_timeout end
    }
    else
    {
        // invalid type
        return 0;
    }

    // "Finally, the timing budget must be re-applied"

    setMeasurementTimingBudget(measurement_timing_budget_us);

    // "Perform the phase calibration. This is needed after changing on vcsel period."
    // VL53L0X_perform_phase_calibration() begin

    uint8_t sequence_config = readReg(SYSTEM_SEQUENCE_CONFIG);
    writeReg(SYSTEM_SEQUENCE_CONFIG, 0x02);
    performSingleRefCalibration(0x0);
    writeReg(SYSTEM_SEQUENCE_CONFIG, sequence_config);

    // VL53L0X_perform_phase_calibration() end

    return 1;
}

// Set the measurement timing budget in microseconds, which is the time allowed
// for one measurement; the ST API and this library take care of splitting the
// timing budget among the sub-steps in the ranging sequence. A longer timing
// budget allows for more accurate measurements. Increasing the budget by a
// factor of N decreases the range measurement standard deviation by a factor of
// sqrt(N). Defaults to about 33 milliseconds; the minimum is 20 ms.
// based on VL53L0X_set_measurement_timing_budget_micro_seconds()
int VL53L0X::setMeasurementTimingBudget(uint32_t budget_us)
{
    uint32_t used_budget_us;
    uint32_t final_range_timeout_us;
    uint16_t final_range_timeout_mclks;

    uint8_t enables;
    SequenceStepTimeouts timeouts;

    uint16_t const StartOverhead      = 1320; // note that this is different than the value in get_
    uint16_t const EndOverhead        = 960;
    uint16_t const MsrcOverhead       = 660;
    uint16_t const TccOverhead        = 590;
    uint16_t const DssOverhead        = 690;
    uint16_t const PreRangeOverhead   = 660;
    uint16_t const FinalRangeOverhead = 550;

    uint32_t const MinTimingBudget = 20000;

    if (budget_us < MinTimingBudget) { return 0; }

    used_budget_us = StartOverhead + EndOverhead;

    enables = readReg(SYSTEM_SEQUENCE_CONFIG);
    getSequenceStepTimeouts(enables, &timeouts);

    if (enables & SEQUENCE_ENABLE_TCC)
    {
        used_budget_us += (timeouts.msrc_dss_tcc_us + TccOverhead);
    }

    if (enables & SEQUENCE_ENABLE_DSS)
    {
        used_budget_us += 2 * (timeouts.msrc_dss_tcc_us + DssOverhead);
    }
    else if (enables & SEQUENCE_ENABLE_MSRC)
    {
        used_budget_us += (timeouts.msrc_dss_tcc_us + MsrcOverhead);
    }

    if (enables & SEQUENCE_ENABLE_PRE_RANGE)
    {
        used_budget_us += (timeouts.pre_range_us + PreRangeOverhead);
    }

    if (enables & SEQUENCE_ENABLE_FINAL_RANGE)
    {
        used_budget_us += FinalRangeOverhead;

        // "Note that the final range timeout is determined by the timing
        // budget and the sum of all other timeouts within the sequence.
        // If there is no room for the final range timeout, then an error
        // will be set. Otherwise the remaining time will be applied to
        // the final range."

        if (used_budget_us > budget_us)
        {
            // "Requested timeout too big."
            return 0;
        }

        final_range_timeout_us = budget_us - used_budget_us;

        // set_sequence_step_timeout() begin
        // (SequenceStepId == VL53L0X_SEQUENCESTEP_FINAL_RANGE)

        // "For the final range timeout, the pre-range timeout
        //  must be added. To do this both final and pre-range
        //  timeouts must be expressed in macro periods MClks
        //  because they have different vcsel periods."

        final_range_timeout_mclks =
                timeoutMicrosecondsToMclks(final_range_timeout_us,
                                           timeouts.final_range_vcsel_period_pclks);

        if (enables & SEQUENCE_ENABLE_PRE_RANGE)
        {
            final_range_timeout_mclks += timeouts.pre_range_mclks;
        }

        writeReg16(FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI,
                   encodeTimeout(final_range_timeout_mclks));

        // set_sequence_step_timeout() end

        measurement_timing_budget_us = budget_us; // store for internal reuse
    }
    return 1;
}

uint32_t VL53L0X::getMeasurementTimingBudget(void)
{
    uint8_t enables;
    SequenceStepTimeouts timeouts;

    uint16_t const StartOverhead     = 1910; // note that this is different than the value in set_
    uint16_t const EndOverhead        = 960;
    uint16_t const MsrcOverhead       = 660;
    uint16_t const TccOverhead        = 590;
    uint16_t const DssOverhead        = 690;
    uint16_t const PreRangeOverhead   = 660;
    uint16_t const FinalRangeOverhead = 550;

    // "Start and end overhead times always present"
    uint32_t budget_us = StartOverhead + EndOverhead;

    enables = readReg(SYSTEM_SEQUENCE_CONFIG);
    getSequenceStepTimeouts(enables, &timeouts);

    if (enables & SEQUENCE_ENABLE_TCC)
    {
        budget_us += (timeouts.msrc_dss_tcc_us + TccOverhead);
    }

    if (enables & SEQUENCE_ENABLE_DSS)
    {
        budget_us += 2 * (timeouts.msrc_dss_tcc_us + DssOverhead);
    }
    else if (enables & SEQUENCE_ENABLE_MSRC)
    {
        budget_us += (timeouts.msrc_dss_tcc_us + MsrcOverhead);
    }

    if (enables & SEQUENCE_ENABLE_PRE_RANGE)
    {
        budget_us += (timeouts.pre_range_us + PreRangeOverhead);
    }

    if (enables & SEQUENCE_ENABLE_FINAL_RANGE)
    {
        budget_us += (timeouts.final_range_us + FinalRangeOverhead);
    }

    measurement_timing_budget_us = budget_us; // store for internal reuse
    return budget_us;
}

int VL53L0X::performSingleRefCalibration(uint8_t vhv_init_byte)
{
    int iTimeout;
    writeReg(SYSRANGE_START, 0x01 | vhv_init_byte); // VL53L0X_REG_SYSRANGE_MODE_START_STOP

    iTimeout = 0;
    while ((readReg(RESULT_INTERRUPT_STATUS) & 0x07) == 0)
    {
        //printf("INTERRUPT: %2.2x, iTimout: %i \n", readReg(RESULT_INTERRUPT_STATUS) & 0x07, iTimeout );
        iTimeout++;
        usleep(5000);
        if (iTimeout > 100) { return 0; }
    }

    writeReg(SYSTEM_INTERRUPT_CLEAR, 0x01);

    writeReg(SYSRANGE_START, 0x00);

    return 1;
} /* performSingleRefCalibration() */

//
// Initialize the vl53l0x
//
int VL53L0X::initSensor(int bLongRangeMode)
{
    unsigned char spad_count=0, spad_type_is_aperture=0, ref_spad_map[6];
    unsigned char ucFirstSPAD, ucSPADsEnabled;
    int i;

    // set 2.8V mode
    writeReg(VHV_CONFIG_PAD_SCL_SDA__EXTSUP_HV,
             readReg(VHV_CONFIG_PAD_SCL_SDA__EXTSUP_HV) | 0x01); // set bit 0
    // Set I2C standard mode
    writeRegList(ucI2CMode);
    stop_variable = readReg(0x91);
    writeRegList(ucI2CMode2);
    // disable SIGNAL_RATE_MSRC (bit 1) and SIGNAL_RATE_PRE_RANGE (bit 4) limit checks
    writeReg(REG_MSRC_CONFIG_CONTROL, readReg(REG_MSRC_CONFIG_CONTROL) | 0x12);
    // Q9.7 fixed point format (9 integer bits, 7 fractional bits)
    writeReg16(FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT, 32); // 0.25
    writeReg(SYSTEM_SEQUENCE_CONFIG, 0xFF);
    getSpadInfo(&spad_count, &spad_type_is_aperture);

    readMulti(GLOBAL_CONFIG_SPAD_ENABLES_REF_0, ref_spad_map, 6);
    printf("initial spad map: %02x,%02x,%02x,%02x,%02x,%02x\n", ref_spad_map[0], ref_spad_map[1], ref_spad_map[2], ref_spad_map[3], ref_spad_map[4], ref_spad_map[5]);
    writeRegList(ucSPAD);
    ucFirstSPAD = (spad_type_is_aperture) ? 12: 0;
    ucSPADsEnabled = 0;
    // clear bits for unused SPADs
    for (i=0; i<48; i++)
    {
        if (i < ucFirstSPAD || ucSPADsEnabled == spad_count)
        {
            ref_spad_map[i>>3] &= ~(1<<(i & 7));
        }
        else if (ref_spad_map[i>>3] & (1<< (i & 7)))
        {
            ucSPADsEnabled++;
        }
    } // for i
    writeMulti(GLOBAL_CONFIG_SPAD_ENABLES_REF_0, ref_spad_map, 6);
    printf("final spad map: %02x,%02x,%02x,%02x,%02x,%02x\n", ref_spad_map[0],
        ref_spad_map[1], ref_spad_map[2], ref_spad_map[3], ref_spad_map[4], ref_spad_map[5]);

    // load default tuning settings
    writeRegList(ucDefTuning); // long list of magic numbers

    // change some settings for long range mode
    if (bLongRangeMode)
    {
        writeReg16(FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT, 13); // 0.1
        setVcselPulsePeriod(VcselPeriodPreRange, 18);
        setVcselPulsePeriod(VcselPeriodFinalRange, 14);
    }

    // set interrupt configuration to "new sample ready"
    writeReg(SYSTEM_INTERRUPT_CONFIG_GPIO, 0x04);
    writeReg(GPIO_HV_MUX_ACTIVE_HIGH, readReg(GPIO_HV_MUX_ACTIVE_HIGH) & ~0x10); // active low
    writeReg(SYSTEM_INTERRUPT_CLEAR, 0x01);
    measurement_timing_budget_us = getMeasurementTimingBudget();
    writeReg(SYSTEM_SEQUENCE_CONFIG, 0xe8);
    setMeasurementTimingBudget(measurement_timing_budget_us);
    writeReg(SYSTEM_SEQUENCE_CONFIG, 0x01);
    if (!performSingleRefCalibration(0x40))
    {
        printf("Error perform first SingleRefCalibration\n");
        return 0;
    }
    writeReg(SYSTEM_SEQUENCE_CONFIG, 0x02);
    if (!performSingleRefCalibration(0x00))
    {
        printf("Error perform second SingleRefCalibration\n");
        return 0;
    }
    writeReg(SYSTEM_SEQUENCE_CONFIG, 0xe8);
    return 1;
} /* initSensor() */

uint16_t VL53L0X::readRangeContinuousMillimeters(void)
{
    int iTimeout = 0;
    uint16_t range;

    while ((readReg(RESULT_INTERRUPT_STATUS) & 0x07) == 0)
    {
        iTimeout++;
        usleep(500);
        if (iTimeout > 50)
        {
            printf("Range timeing out!\n");
            return -1;
        }
    }

    // assumptions: Linearity Corrective Gain is 1000 (default);
    // fractional ranging is not enabled
    range = readReg16(RESULT_RANGE_STATUS + 10);

    writeReg(SYSTEM_INTERRUPT_CLEAR, 0x01);

    return range;
}
//
// Read the current distance in mm
//
int VL53L0X::tofReadDistance(void)
{
    int iTimeout;

    writeReg(0x80, 0x01);
    writeReg(0xFF, 0x01);
    writeReg(0x00, 0x00);
    writeReg(0x91, stop_variable);
    writeReg(0x00, 0x01);
    writeReg(0xFF, 0x00);
    writeReg(0x80, 0x00);

    writeReg(SYSRANGE_START, 0x01);

    // "Wait until start bit has been cleared"
    iTimeout = 0;
    while (readReg(SYSRANGE_START) & 0x01)
    {
        iTimeout++;
        usleep(500);
        if (iTimeout > 50)
        {
            printf("VL53L0X::tofReadDistance(void) error starting.\n");
            return -1;
        }
    }
    int val = readRangeContinuousMillimeters();
    printf("Distance: %i mm \n",val);
    return val;
} /* tofReadDistance() */

int VL53L0X::tofGetModel(int *model, int *revision)
{
    if (model)
    {
        *model = readReg(REG_IDENTIFICATION_MODEL_ID);
    }
    if (revision)
    {
        *revision = readReg(REG_IDENTIFICATION_REVISION_ID);
    }

    return 1;
} /* tofGetModel() */

#endif
