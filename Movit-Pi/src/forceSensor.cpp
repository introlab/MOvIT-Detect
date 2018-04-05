//---------------------------------------------------------------------------------------
// HEADER DE FICHIER
// Description
//---------------------------------------------------------------------------------------

#include "MAX11611.h"    //10-Bit ADC
#include "forceSensor.h" //variables and modules initialisation

#include <stdio.h>
#include <unistd.h>

forceSensor::forceSensor()
{
  for (uint8_t i = 0; i < _sensorCount; i++)
  {
    _analogData[i] = 0;
    _voltageData[i] = 0;
    _resistanceData[i] = 0;
    _conductanceData[i] = 0;
    _forceData[i] = 0;
    _analogOffset[i] = 0;
  }
  _totalSensorMean = 0;
  _detectionThreshold = 0;
}

forceSensor::~forceSensor() {}

//---------------------------------------------------------------------------------------
//Function: ForceSensor::CalibrateForceSensor
//Force sensor individual calibration - establish initial offset
//Used by presence detection and center of pressure displacement functions
//---------------------------------------------------------------------------------------
void forceSensor::CalibrateForceSensor(uint16_t *max11611Data, MAX11611 &max11611)
{
  /***************************** FRS MAP *****************************/
  /* FRONT LEFT                                          FRONT RIGHT */
  /* max11611Data[7]         max11611Data[4]         max11611Data[1] */
  /* max11611Data[8]         max11611Data[5]         max11611Data[2] */
  /* max11611Data[9]         max11611Data[6]         max11611Data[3] */
  /*******************************************************************/

  const float calibrationRatio = 0.75;
  const uint8_t maxIterations = 10;  //Calibration preceision adjustement - Number of measures (1s) in final mean
  uint16_t sensorMean[_sensorCount]; //Individual iterations sensors mean
  uint32_t totalSensorMean = 0;      //Final sensors analog data reading mean

  //Sensor mean table initialization
  for (uint8_t i = 0; i < _sensorCount; i++)
  {
    sensorMean[i] = 0;
  }

  //Mean generation for calibration operation
  for (uint8_t i = 0; i < maxIterations; i++)
  {
    //Update sensor analog data readings
    printf("\n%i ", (maxIterations - i));
    max11611.getData(_sensorCount, max11611Data);

    //Force analog data readings mean
    for (uint8_t j = 0; j < _sensorCount; j++)
    {
      SetAnalogData(max11611Data[j], j);
      sensorMean[j] += GetAnalogData(j);

      if (i == maxIterations - 1 && maxIterations != 0)
      {
        sensorMean[j] /= maxIterations;
      }
    }
    delay(1000);
  }
  printf("\nDONE\n\n");

  //Total sensors analog data readings mean
  for (uint8_t i = 0; i < _sensorCount; i++)
  {
    SetAnalogOffset(sensorMean[i], i);
    totalSensorMean += sensorMean[i];
  }
  if (maxIterations != 0)
  {
    totalSensorMean /= maxIterations;
  }
  SetTotalSensorMean(totalSensorMean);
  SetDetectionThreshold(calibrationRatio * GetTotalSensorMean());
}

//---------------------------------------------------------------------------------------
//Function: ForceSensorUnits
//Force plate creation from 2x2 force sensors matrix
//Centor of Pressure calculation and Coefficient of Friction
//Reference: Kistler force plate formulae PDF
//---------------------------------------------------------------------------------------
void forceSensor::GetForceSensorData()
{
  /***************************** FRS MAP *****************************/
  /* FRONT LEFT                                          FRONT RIGHT */
  /* max11611Data[7]         max11611Data[4]         max11611Data[1] */
  /* max11611Data[8]         max11611Data[5]         max11611Data[2] */
  /* max11611Data[9]         max11611Data[6]         max11611Data[3] */
  /*******************************************************************/

  for (uint8_t i = 0; i < _sensorCount; i++)
  {
    //Sensor data - Voltage (mV)
    uint32_t voltValue = GetAnalogData(i) * 5000 / 1023;
    SetVoltageData(voltValue, i);

    //Sensor data - Resistance (uOhm)
    //The tension = Vcc * R / (R + FSR)
    //FSR = ((Vcc - V) * R) / V
    if (GetVoltageData(i) != 0)
    {
      uint32_t resValue = ((5000 - GetVoltageData(i)) * 10000) / GetVoltageData(i);
      SetResistanceData(resValue, i);
    }
    else
    {
      SetResistanceData(0, i);
    }
    //Sensor data - Conductance
    if (GetResistanceData(i) != 0)
    {
      SetConductanceData(1000000 / GetResistanceData(i), i); // we measure in micromhos so
    }
    else
    {
      SetConductanceData(0, i);
    }
    //Sensor data - Force (N)
    float forceValue = 0.0f;
    if (GetConductanceData(i) <= 1000 && GetConductanceData(i) != 0)
    {
      forceValue = float(GetConductanceData(i)) / 80.0;
    }
    else if (GetConductanceData(i) > 1000 && GetConductanceData(i) != 0)
    {
      forceValue = (float(GetConductanceData(i)) - 1000.0) / 30.0;
    }
    else
    {
      forceValue = 0.0f;
    }
    SetForceData(forceValue, i);
  }
}

//---------------------------------------------------------------------------------------
//Function: ForceSensor::IsUserDetected
//
//---------------------------------------------------------------------------------------
bool forceSensor::IsUserDetected()
{
  /***************************** FRS MAP *****************************/
  /* FRONT LEFT                                          FRONT RIGHT */
  /* max11611Data[7]         max11611Data[4]         max11611Data[1] */
  /* max11611Data[8]         max11611Data[5]         max11611Data[2] */
  /* max11611Data[9]         max11611Data[6]         max11611Data[3] */
  /*******************************************************************/

  //Total of all sensors reading analog data
  float sensedPresence = 0;
  for (uint8_t i = 0; i < _sensorCount; i++)
  {
    sensedPresence += float(GetAnalogData(i));
  }
  if (_sensorCount != 0)
  {
    sensedPresence /= _sensorCount;
  }
  return (sensedPresence > GetDetectionThreshold());
}

int *forceSensor::DetectRelativePressure()
{
  /*********FORCE PLATES MAP *********** y ********* */
  /* FRONT LEFT           FRONT RIGHT    |           */
  /* Quadrant1            Quadrant2      |           */
  /* Quadrant3            Quadrant4      |------->x  */
  /***************************************************/

  //Quadrant reference and current (mean values)
  const uint8_t quadrantCount = 4;
  uint16_t referenceQuadrant[quadrantCount];
  uint16_t currentQuadrant[quadrantCount];
  static int relativePressureLevel[quadrantCount]; //return value

  //Threshold levels (5) : really low (RL), low (L), normal (N), high (H), really high (RH)
  const float thresholdFactorRL = 0.25;
  const float thresholdFactorL = 0.5;
  const float thresholdFactorH = 1.5;
  const float thresholdFactorRH = 1.75;
  float quadrantRLThreshold[quadrantCount];
  float quadrantLThreshold[quadrantCount];
  float quadrantHThreshold[quadrantCount];
  float quadrantRHThreshold[quadrantCount];

  //Quadrants reference values from calibration analog offset measures (mean values)
  referenceQuadrant[0] = (GetAnalogOffset(4) + GetAnalogOffset(1) + GetAnalogOffset(0) + GetAnalogOffset(3)) / 4;
  referenceQuadrant[1] = (GetAnalogOffset(7) + GetAnalogOffset(4) + GetAnalogOffset(3) + GetAnalogOffset(6)) / 4;
  referenceQuadrant[2] = (GetAnalogOffset(5) + GetAnalogOffset(2) + GetAnalogOffset(1) + GetAnalogOffset(4)) / 4;
  referenceQuadrant[3] = (GetAnalogOffset(8) + GetAnalogOffset(5) + GetAnalogOffset(4) + GetAnalogOffset(7)) / 4;

  //Pressure levels detection threshold generation from calibration analog offset measures
  for (uint8_t i = 0; i < quadrantCount; i++)
  {
    relativePressureLevel[i] = 0;
    quadrantRLThreshold[i] = thresholdFactorRL * referenceQuadrant[i]; //Really low pressure in quadrant i
    quadrantLThreshold[i] = thresholdFactorL * referenceQuadrant[i];   //Low pressure in quadrant i                     //Normal pressure in quadrant i
    quadrantHThreshold[i] = thresholdFactorH * referenceQuadrant[i];   //High pressure in quadrant i
    quadrantRHThreshold[i] = thresholdFactorRH * referenceQuadrant[i]; //Really high pressure in quadrant i
  }

  //Quadrants current values (mean values)
  currentQuadrant[0] = (GetAnalogData(4) + GetAnalogData(1) + GetAnalogData(0) + GetAnalogData(3)) / 4;
  currentQuadrant[1] = (GetAnalogData(7) + GetAnalogData(4) + GetAnalogData(3) + GetAnalogData(6)) / 4;
  currentQuadrant[2] = (GetAnalogData(5) + GetAnalogData(2) + GetAnalogData(1) + GetAnalogData(4)) / 4;
  currentQuadrant[3] = (GetAnalogData(8) + GetAnalogData(5) + GetAnalogData(4) + GetAnalogData(7)) / 4;

  //Compare current quadrant data with threshold
  for (uint8_t i = 0; i < quadrantCount; i++)
  {
    // //Comment-Uncomment for debug
    // printf("\n Quadrant : %i \n", i);
    // printf("referenceQuadrant : %i \n", referenceQuadrant[i]);
    // printf("quadrantRLThreshold : %f \n", quadrantRLThreshold[i]);
    // printf("quadrantLThreshold : %f \n", quadrantLThreshold[i]);
    // printf("quadrantHThreshold : %f \n", quadrantHThreshold[i]);
    // printf("quadrantRHThreshold : %f \n", quadrantRHThreshold[i]);
    // printf("currentQuadrant : %i \n", currentQuadrant[i]);

    if (quadrantRLThreshold[i] >= currentQuadrant[i] && currentQuadrant[i] > 0)
    {
      relativePressureLevel[i] = 1; //Really low pressure in quadrant i
    }
    else if (quadrantLThreshold[i] >= currentQuadrant[i] && currentQuadrant[i] > quadrantRLThreshold[i])
    {
      relativePressureLevel[i] = 2; //Low pressure in quadrant i
    }
    else if (quadrantHThreshold[i] >= currentQuadrant[i] && currentQuadrant[i] > quadrantLThreshold[i])
    {
      relativePressureLevel[i] = 3; //Normal pressure in quadrant i
    }
    else if (quadrantRHThreshold[i] >= currentQuadrant[i] && currentQuadrant[i] > quadrantHThreshold[i])
    {
      relativePressureLevel[i] = 4; //High pressure in quadrant i
    }
    else if (15000 >= currentQuadrant[i] && currentQuadrant[i] > quadrantHThreshold[i])
    {
      relativePressureLevel[i] = 5; //High pressure in quadrant i
    }
    else if (currentQuadrant[i] == 0)
    {
      relativePressureLevel[i] = 0;
    }
    else
    {
      //Reading error
    }
  }

  return relativePressureLevel;
}
