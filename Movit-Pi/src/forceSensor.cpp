//---------------------------------------------------------------------------------------
// HEADER DE FICHIER
// Description
//---------------------------------------------------------------------------------------

#include "MAX11611.h"  //10-Bit ADC
#include "forceSensor.h"   //variables and modules initialisation

extern MAX11611 max11611;                    //Initialisation of the 10-bit ADC
const uint8_t capteurForceNb = 9;                //Total number of sensors
extern uint16_t max11611Data[capteurForceNb];     //Data table of size=total sensors
extern uint16_t max11611EmptyData[capteurForceNb];     //Data table of size=total sensors
long sensedPresence;
long detectionThreshold;


void forceSensor::CalibrateForceSensor(forceSensor &sensors)
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
  uint16_t sensorMean[sensors.sensorCount];    //Individual iterations sensors mean
  uint16_t totalSensorMean;                          //Final sensors analog data reading mean

  //Sensor mean table initialization
  for(int i = 0; i < sensors.sensorCount; i++)
  {
    sensorMean[i] = max11611EmptyData[i];
  }

  //Mean generation for calibration operation
  for(int i = 0; i < maxIterations; i++)
  {
    delay(1000);
    printf("%i", (maxIterations-i));
    printf("\n");

    //Update sensor analog data readings
    max11611.getData(sensors.sensorCount, max11611Data);

    //Force analog data readings mean
    for(int j = 0; j < sensors.sensorCount; j++)
    {
      sensors.SetAnalogData(max11611Data[j], j);
      sensorMean[j] += sensors.GetAnalogData(j);

      if(i == maxIterations - 1)
      {
        sensorMean[j] /= maxIterations;
      }
    }
   }

   //Total sensors analog data readings mean
   for(int i = 0; i < sensors.sensorCount; i++)
   {
      sensors.SetAnalogOffset(sensorMean[i], i);
      totalSensorMean += sensorMean[i];
   }
   totalSensorMean /= maxIterations;
   sensors.SetTotalSensorMean(totalSensorMean);
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
    // //Sensor data - Voltage (mV)
    // long voltValue = map(sensors.GetAnalogData(i), 0 , 1023, 0, 5000);
    // sensors.SetVoltageData(voltValue, i);
    //
    // //Sensor data - Resistance (uOhm)
    // //The tension = Vcc * R / (R + FSR)
    // //FSR = ((Vcc - V) * R) / V
    // long resValue =  5000 - sensors.GetVoltageData(i);
    // resValue *= 10000;
    // resValue /= sensors.GetVoltageData(i);
    // sensors.SetResistanceData(resValue, i);
    //
    // //Sensor data - Conductance
    // long condValue = 1000000;           // we measure in micromhos so
    // condValue /= sensors.GetResistanceData(i);
    // sensors.SetConductanceData(condValue, i);
    //
    // //Sensor data - Force (N)
    // float forceValue;
    // if (sensors.GetConductanceData(i) <= 1000)
    // {
    //   forceValue = sensors.GetConductanceData(i)/80;
    // }
    // else
    // {
    //   forceValue = sensors.GetConductanceData(i) - 1000;
    //   forceValue /= 30;
    // }
    // sensors.SetForceData(forceValue, i);
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

  sensedPresence = 0;
  detectionThreshold = calibrationRatio * sensors.GetTotalSensorMean();

  //Total of all sensors reading analog data
  for (int i = 0; i < sensors.sensorCount; i++)
  {
    sensedPresence += sensors.GetAnalogData(i);
  }
  sensedPresence /= sensors.sensorCount;
  return (sensedPresence >= detectionThreshold);
}
