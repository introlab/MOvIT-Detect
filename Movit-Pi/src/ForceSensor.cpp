#include "MAX11611.h"  //10-Bit ADC
#include "ForceSensor.h"
#include <unistd.h>
#include "Utils.h"

//External functions and variables
extern MAX11611 max11611;                    //Initialisation of the 10-bit ADC
extern uint16_t max11611Data[sensorCount];     //Data table of size=total sensors
extern long detectedPresence;
extern long detectionThreshold;

const float forceCalibrationRatio = 0.75;

//---------------------------------------------------------------------------------------
//Function: ForceSensor::CalibrateForceSensor
//Force sensor individual calibration - establish initial offset
//Used by presence detection and center of pressure displacement functions
//---------------------------------------------------------------------------------------
void ForceSensor::CalibrateForceSensor(ForceSensor &sensors)
{
  /***************************** FRS MAP *****************************/
  /* FRONT LEFT                                          FRONT RIGHT */
  /* max11611Data[7]         max11611Data[4]         max11611Data[1] */
  /* max11611Data[8]         max11611Data[5]         max11611Data[2] */
  /* max11611Data[9]         max11611Data[6]         max11611Data[3] */
  /*******************************************************************/

  const int maxIterations = 10;    //Calibration preceision adjustement - Number of measures (1s) in final mean
  uint16_t sensorMean[sensorCount];    //Individual iterations sensors mean
  uint16_t totalSensorMean;                          //Final sensors analog data reading mean

  //Sensor mean table initialization
  for(int i = 0; i < sensorCount; i++)
  {
    sensorMean[i] = 0;
  }

  //Mean generation for calibration operation
  for(int i = 0; i < maxIterations; i++)
  {
    usleep(500000);
    printf("%i", maxIterations-i);;

    //Update sensor analog data readings
    max11611.getData(sensorCount, max11611Data);

    //Force analog data readings mean
    for(int j = 0; j < sensorCount; j++)
    {
      sensors.setAnalogData(max11611Data[j], i);
      sensorMean[j] += sensors.getAnalogData(j);
      sensorMean[j] /= 2;
    }
   }

   //Total sensors analog data readings mean
   for(int i = 0; i < sensorCount; i++)
   {
     sensors.setAnalogOffset(sensorMean[i], i);
     totalSensorMean += sensorMean[i];
     totalSensorMean /= 2;
   }
   sensors.setTotalSensorMean(totalSensorMean);
}

void ForceSensor::GetForceSensorData(ForceSensor &sensors)
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
    long voltValue;
    voltValue = map(sensors.getAnalogData(i), 0 , 1023, 0, 5000);
    sensors.setVoltageData(voltValue, i);

    //Sensor data - Resistance (uOhm)
    //The tension = Vcc * R / (R + FSR)
    //FSR = ((Vcc - V) * R) / V
    long resValue;
    resValue =  5000 - sensors.getVoltageData(i);
    resValue *= 10000;
    // Possible division par 0
    resValue /= sensors.getVoltageData(i);
    sensors.setResistanceData(resValue, i);

    //Sensor data - Conductance
    long condValue;
    condValue = 1000000;           // we measure in micromhos so
    condValue /= sensors.getResistanceData(i);
    sensors.setConductanceData(condValue, i);

    //Sensor data - Force (N)
    double forceValue;
    if (sensors.getConductanceData(i) <= 1000)
    {
      forceValue = sensors.getConductanceData(i)/80;
    }
    else
    {
      forceValue = sensors.getConductanceData(i) - 1000;
      forceValue /= 30;
    }
    sensors.setForceData(forceValue, i);
  }
}

bool ForceSensor::isPresenceDetected(ForceSensor &sensors)
//---------------------------------------------------------------------------------------
//Function: ForceSensor::isPresenceDetected
//
//---------------------------------------------------------------------------------------
{
  /***************************** FRS MAP *****************************/
  /* FRONT LEFT                                          FRONT RIGHT */
  /* max11611Data[7]         max11611Data[4]         max11611Data[1] */
  /* max11611Data[8]         max11611Data[5]         max11611Data[2] */
  /* max11611Data[9]         max11611Data[6]         max11611Data[3] */
  /*******************************************************************/

  detectedPresence = 0;
  detectionThreshold = forceCalibrationRatio * sensors.getTotalSensorMean();

  //Total of all sensors reading analog data
  for (int i = 0; i < sensorCount; i++)
  {
    detectedPresence += sensors.getAnalogData(i);
    detectedPresence /= 2;
  }
  return (detectedPresence >= detectionThreshold);
}
