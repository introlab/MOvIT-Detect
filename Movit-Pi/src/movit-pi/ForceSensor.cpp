//---------------------------------------------------------------------------------------
// HEADER DE FICHIER
// Description
//---------------------------------------------------------------------------------------

#include "MAX11611.h"    //10-Bit ADC
#include "ForceSensor.h" //variables and modules initialisation
#include "GlobalForcePlate.h"

#include <stdio.h>
#include <unistd.h>

ForceSensor::ForceSensor()
{
    for (uint8_t i = 0; i < PRESSURE_SENSOR_COUNT; i++)
    {
        _analogData[i] = 0;
        _analogOffset[i] = 0;
    }
    _totalSensorMean = 0;
    _detectionThreshold = 0;
}

ForceSensor::~ForceSensor() {}

pressure_mat_offset_t ForceSensor::GetOffsets()
{
    pressure_mat_offset_t ret;

    for (int i = 0; i < PRESSURE_SENSOR_COUNT; i++)
    {
        ret.analogOffset[i] = _analogOffset[i];
    }
    ret.detectionThreshold = _detectionThreshold;
    ret.totalSensorMean = _totalSensorMean;

    return ret;
}

void ForceSensor::SetOffsets(pressure_mat_offset_t offset)
{
    for (int i = 0; i < PRESSURE_SENSOR_COUNT; i++)
    {
        _analogOffset[i] = offset.analogOffset[i];
    }
    _detectionThreshold = offset.detectionThreshold;
    _totalSensorMean = offset.totalSensorMean;
}

//---------------------------------------------------------------------------------------
//Function: ForceSensor::CalibrateForceSensor
//Force sensor individual calibration - establish initial offset
//Used by presence detection and center of pressure displacement functions
//---------------------------------------------------------------------------------------
void ForceSensor::CalibrateForceSensor(MAX11611 &max11611, uint16_t *max11611Data, uint8_t maxIterations)
{
    /***************************** FRS MAP *****************************/
    /* FRONT LEFT                                          FRONT RIGHT */
    /* max11611Data[7]         max11611Data[4]         max11611Data[1] */
    /* max11611Data[8]         max11611Data[5]         max11611Data[2] */
    /* max11611Data[9]         max11611Data[6]         max11611Data[3] */
    /*******************************************************************/
    if (!maxIterations)
    {
        printf("Error: Invalid iteration number.\n");
        return;
    }

    const float calibrationRatio = 0.75;
    uint16_t sensorMean[PRESSURE_SENSOR_COUNT]; //Individual iterations sensors mean
    _totalSensorMean = 0;                       //Final sensors analog data reading mean

    //Sensor mean table initialization
    for (uint8_t i = 0; i < PRESSURE_SENSOR_COUNT; i++)
    {
        sensorMean[i] = 0;
    }

    printf("\nCalculating ... ");
    //Mean generation for calibration operation
    for (uint8_t i = 0; i < maxIterations; i++)
    {
        //Update sensor analog data readings
        printf("\n%i ", (maxIterations - i));
        _max11611.GetData(PRESSURE_SENSOR_COUNT, _max11611Data);
        for (uint8_t i = 0; i < PRESSURE_SENSOR_COUNT; i++)
        {
            SetAnalogData(i, _max11611Data[i]);
        }
        //Force analog data readings mean
        for (uint8_t j = 0; j < PRESSURE_SENSOR_COUNT; j++)
        {
            sensorMean[j] += GetAnalogData(j);

            if (i == maxIterations - 1)
            {
                sensorMean[j] /= maxIterations;
            }
        }
        delay(1000);
    }

    //Total sensors analog data readings mean
    for (uint8_t i = 0; i < PRESSURE_SENSOR_COUNT; i++)
    {
        _analogOffset[i] = sensorMean[i];
        _totalSensorMean += sensorMean[i];
    }
    _totalSensorMean /= maxIterations;
    _detectionThreshold = calibrationRatio * _totalSensorMean;
    printf("\ntotalmeansen = %i \n", _totalSensorMean);
    printf("\ndetectionThreshold = %f\n", _detectionThreshold);

    printf("\nDONE\n");
}

//---------------------------------------------------------------------------------------
//Function: ForceSensor::IsUserDetected
//
//---------------------------------------------------------------------------------------
bool ForceSensor::IsUserDetected()
{
    /***************************** FRS MAP *****************************/
    /* FRONT LEFT                                          FRONT RIGHT */
    /* max11611Data[7]         max11611Data[4]         max11611Data[1] */
    /* max11611Data[8]         max11611Data[5]         max11611Data[2] */
    /* max11611Data[9]         max11611Data[6]         max11611Data[3] */
    /*******************************************************************/

    //Total of all sensors reading analog data
    float sensedPresence = 0;
    for (uint8_t i = 0; i < PRESSURE_SENSOR_COUNT; i++)
    {
        sensedPresence += static_cast<float>(_analogData[i]);
    }
    if (!PRESSURE_SENSOR_COUNT)
    {
        sensedPresence /= PRESSURE_SENSOR_COUNT;
    }
    return sensedPresence > _detectionThreshold;
}

uint16_t ForceSensor::GetAnalogData(uint8_t index)
{
    //Rearrange analog data to fix a hardware problem.
    uint16_t rearrangedAnalogData[PRESSURE_SENSOR_COUNT];
    rearrangedAnalogData[0] = _analogData[5];
    rearrangedAnalogData[1] = _analogData[7];
    rearrangedAnalogData[2] = _analogData[6];
    rearrangedAnalogData[3] = _analogData[2];
    rearrangedAnalogData[4] = _analogData[4];
    rearrangedAnalogData[5] = _analogData[3];
    rearrangedAnalogData[6] = _analogData[1];
    rearrangedAnalogData[7] = _analogData[0];
    rearrangedAnalogData[8] = _analogData[8];

    return rearrangedAnalogData[index];
}
