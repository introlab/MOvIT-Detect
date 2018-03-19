//---------------------------------------------------------------------------------------
// HEADER DE FICHIER
// Description
//---------------------------------------------------------------------------------------

#include "MAX11611.h"  //10-Bit ADC
#include "forceSensor.h"   //variables and modules initialisation

#include <stdio.h>
#include <unistd.h>

extern MAX11611 max11611;                    //Initialisation of the 10-bit ADC

forceSensor::forceSensor()
{
  for(int i = 0; i < sensorCount; i++)
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

void forceSensor::CalibrateForceSensor(uint16_t* max11611Data, forceSensor &sensors)
//---------------------------------------------------------------------------------------
//Function: ForceSensor::CalibrateForceSensor
//Force sensor individual calibration - establish initial offset
//Used by presence detection and center of pressure displacement functions
//---------------------------------------------------------------------------------------
{
  /***************************** FRS MAP *****************************/
  /* FRONT LEFT                                          FRONT RIGHT */
  /* max11611Data[7]         max11611Data[4]         max11611Data[1] */
  /* max11611Data[8]         max11611Data[5]         max11611Data[2] */
  /* max11611Data[9]         max11611Data[6]         max11611Data[3] */
  /*******************************************************************/

  const int maxIterations = 10;    //Calibration preceision adjustement - Number of measures (1s) in final mean
  uint16_t sensorMean[sensorCount];    //Individual iterations sensors mean
  uint16_t totalSensorMean = 0;                          //Final sensors analog data reading mean

  //Sensor mean table initialization
  for(int i = 0; i < sensorCount; i++)
  {
    sensorMean[i] = 0;
  }

  //Mean generation for calibration operation
  for(int i = 0; i < maxIterations; i++)
  {
    //Update sensor analog data readings
    printf("\n%i ", (maxIterations-i));
    max11611.getData(sensorCount, max11611Data);

    //Force analog data readings mean
    for(int j = 0; j < sensorCount; j++)
    {
      sensors.SetAnalogData(max11611Data[j], j);
      sensorMean[j] += sensors.GetAnalogData(j);

      if(i == maxIterations - 1 && maxIterations != 0)
      {
        sensorMean[j] /= maxIterations;
      }
    }
    delay(1000);
   }
   printf("\nDONE\n\n");

   //Total sensors analog data readings mean
   for(int i = 0; i < sensorCount; i++)
   {
      sensors.SetAnalogOffset(sensorMean[i], i);
      totalSensorMean += sensorMean[i];
   }
   if(maxIterations != 0)
   {
     totalSensorMean /= maxIterations;
   }
   sensors.SetTotalSensorMean(totalSensorMean);
   sensors.SetDetectionThreshold(calibrationRatio * sensors.GetTotalSensorMean());
}

void forceSensor::GetForceSensorData(forceSensor &sensors)
//---------------------------------------------------------------------------------------
//Function: ForceSensorUnits
//Force plate creation from 2x2 force sensors matrix
//Centor of Pressure calculation and Coefficient of Friction
//Reference: Kistler force plate formulae PDF
//---------------------------------------------------------------------------------------
{
  /***************************** FRS MAP *****************************/
  /* FRONT LEFT                                          FRONT RIGHT */
  /* max11611Data[7]         max11611Data[4]         max11611Data[1] */
  /* max11611Data[8]         max11611Data[5]         max11611Data[2] */
  /* max11611Data[9]         max11611Data[6]         max11611Data[3] */
  /*******************************************************************/

  for (int i = 0; i < sensorCount; i++)
  {
    //Sensor data - Voltage (mV)
    long voltValue = sensors.GetAnalogData(i)*5000/1023;
    sensors.SetVoltageData(voltValue, i);

    //Sensor data - Resistance (uOhm)
    //The tension = Vcc * R / (R + FSR)
    //FSR = ((Vcc - V) * R) / V
    long resValue =  5000 - sensors.GetVoltageData(i);
    resValue *= 10000;
    if (sensors.GetVoltageData(i) != 0)
    {
      resValue /= sensors.GetVoltageData(i);
      sensors.SetResistanceData(resValue, i);
    }
    else
    {
      sensors.SetResistanceData(0, i);
    }
    //Sensor data - Conductance
    long condValue = 1000000;           // we measure in micromhos so
    if (sensors.GetResistanceData(i) != 0)
    {
      condValue /= sensors.GetResistanceData(i);
      sensors.SetConductanceData(condValue, i);
    }
    else
    {
      sensors.SetConductanceData(0, i);
    }
    //Sensor data - Force (N)
    float forceValue;
    if (sensors.GetConductanceData(i) <= 1000 && sensors.GetConductanceData(i) != 0)
    {
      forceValue = sensors.GetConductanceData(i)/80;
    }
    else if (sensors.GetConductanceData(i) > 1000 && sensors.GetConductanceData(i) != 0)
    {
      forceValue = sensors.GetConductanceData(i) - 1000;
      forceValue /= 30;
    }
    else
    {
      forceValue = 0;
    }
    sensors.SetForceData(forceValue, i);
  }
}

bool forceSensor::IsUserDetected(forceSensor &sensors)
//---------------------------------------------------------------------------------------
//Function: ForceSensor::IsUserDetected
//
//---------------------------------------------------------------------------------------
{
  /***************************** FRS MAP *****************************/
  /* FRONT LEFT                                          FRONT RIGHT */
  /* max11611Data[7]         max11611Data[4]         max11611Data[1] */
  /* max11611Data[8]         max11611Data[5]         max11611Data[2] */
  /* max11611Data[9]         max11611Data[6]         max11611Data[3] */
  /*******************************************************************/

  long sensedPresence = 0;

  //Total of all sensors reading analog data
  for (int i = 0; i < sensors.sensorCount; i++)
  {
    sensedPresence += sensors.GetAnalogData(i);
  }
  if(sensors.sensorCount != 0)
  {
    sensedPresence /= sensors.sensorCount;
  }
  return (sensedPresence >= sensors.GetDetectionThreshold());
}

int *forceSensor::DetectRelativePressure(forceSensor &sensors)
{
  /*********FORCE PLATES MAP *********** y ********* */
  /* FRONT LEFT           FRONT RIGHT    |           */
  /* Quadrant1            Quadrant2      |           */
  /* Quadrant3            Quadrant4      |------->x  */
  /***************************************************/

  //Quadrant reference and current (mean values)
  const int quadrantCount = 4;
  uint16_t referenceQuadrant[quadrantCount];
  uint16_t currentQuadrant[quadrantCount];
  static int relativePressureLevel[quadrantCount]; //return value

  //Threshold levels (5) : really low (RL), low (L), normal (N), high (H), really high (RH)
  float thresholdFactorRL = 0.25;
  float thresholdFactorL = 0.5;
  float thresholdFactorH = 1.5;
  float thresholdFactorRH = 1.75;
  float quadrantRLThreshold[quadrantCount];
  float quadrantLThreshold[quadrantCount];
  float quadrantHThreshold[quadrantCount];
  float quadrantRHThreshold[quadrantCount];

  //Quadrants reference values from calibration analog offset measures (mean values)
  referenceQuadrant[0] = (sensors.GetAnalogOffset(4) + sensors.GetAnalogOffset(1) + sensors.GetAnalogOffset(0) + sensors.GetAnalogOffset(3)) / 4;
  referenceQuadrant[1] = (sensors.GetAnalogOffset(7) + sensors.GetAnalogOffset(4) + sensors.GetAnalogOffset(3) + sensors.GetAnalogOffset(6)) / 4;
  referenceQuadrant[2] = (sensors.GetAnalogOffset(5) + sensors.GetAnalogOffset(2) + sensors.GetAnalogOffset(1) + sensors.GetAnalogOffset(4)) / 4;
  referenceQuadrant[3] = (sensors.GetAnalogOffset(8) + sensors.GetAnalogOffset(5) + sensors.GetAnalogOffset(4) + sensors.GetAnalogOffset(7)) / 4;

  //Pressure levels detection threshold generation from calibration analog offset measures
  for (int i = 0; i < quadrantCount; i++)
  {
    relativePressureLevel[i] = 0;
    quadrantRLThreshold[i] = thresholdFactorRL * referenceQuadrant[i];    //Really low pressure in quadrant i
    quadrantLThreshold[i] = thresholdFactorL * referenceQuadrant[i];      //Low pressure in quadrant i                     //Normal pressure in quadrant i
    quadrantHThreshold[i] = thresholdFactorH * referenceQuadrant[i];      //High pressure in quadrant i
    quadrantRHThreshold[i] = thresholdFactorRH * referenceQuadrant[i];    //Really high pressure in quadrant i
  }

  //Quadrants current values (mean values)
  currentQuadrant[0] = (sensors.GetAnalogData(4) + sensors.GetAnalogData(1) + sensors.GetAnalogData(0) + sensors.GetAnalogData(3)) / 4;
  currentQuadrant[1] = (sensors.GetAnalogData(7) + sensors.GetAnalogData(4) + sensors.GetAnalogData(3) + sensors.GetAnalogData(6)) / 4;
  currentQuadrant[2] = (sensors.GetAnalogData(5) + sensors.GetAnalogData(2) + sensors.GetAnalogData(1) + sensors.GetAnalogData(4)) / 4;
  currentQuadrant[3] = (sensors.GetAnalogData(8) + sensors.GetAnalogData(5) + sensors.GetAnalogData(4) + sensors.GetAnalogData(7)) / 4;

  //Compare current quadrant data with threshold
  for (int i = 0; i < quadrantCount; i++)
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
      relativePressureLevel[i] = 1;  //Really low pressure in quadrant i
    }
    else if (quadrantLThreshold[i] >= currentQuadrant[i] && currentQuadrant[i] > quadrantRLThreshold[i])
    {
      relativePressureLevel[i] = 2;  //Low pressure in quadrant i
    }
    else if (quadrantHThreshold[i] >= currentQuadrant[i] && currentQuadrant[i] > quadrantLThreshold[i])
    {
      relativePressureLevel[i] = 3;  //Normal pressure in quadrant i
    }
    else if (quadrantRHThreshold[i] >= currentQuadrant[i] && currentQuadrant[i] > quadrantHThreshold[i])
    {
      relativePressureLevel[i] = 4;  //High pressure in quadrant i
    }
    else if (15000 >= currentQuadrant[i] && currentQuadrant[i] > quadrantHThreshold[i])
    {
      relativePressureLevel[i] = 5;  //High pressure in quadrant i
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
