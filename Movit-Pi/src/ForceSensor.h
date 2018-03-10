#ifndef _FORCESENSOR_H_
#define _FORCESENSOR_H_

//Constants - physical montage values
const uint8_t sensorCount = 9; //Total number of sensor in the matrix

class ForceSensor
{
public:
  void CalibrateForceSensor(ForceSensor &sensors);
  void GetForceSensorData(ForceSensor &sensors);
  bool isPresenceDetected(ForceSensor &sensors);

  uint16_t getAnalogData(int index) { return _analogData[index]; }
  long getVoltageData(int index) { return _voltageData[index]; };
  long getResistanceData(int index) { return _resistanceData[index]; };
  long getConductanceData(int index) { return _conductanceData[index]; };
  double getForceData(int index) { return _forceData[index]; };
  long getAnalogOffset(int index) { return _analogOffset[index]; };
  int getTotalSensorMean() { return _totalSensorMean; };

  void setAnalogData(uint16_t analogdata, int index) { _analogData[index] = analogdata; }
  void setVoltageData(uint16_t voltageData, int index) { _voltageData[index] = voltageData; }
  void setResistanceData(uint16_t resistanceData, int index) { _resistanceData[index] = resistanceData; }
  void setConductanceData(uint16_t conductanceData, int index) { _conductanceData[index] = conductanceData; }
  void setForceData(float forceData, int index) { _forceData[index] = forceData; }
  void setAnalogOffset(uint16_t analogOffset, int index) { _analogOffset[index] = analogOffset; }
  void setTotalSensorMean(int totalSensorMean) { _totalSensorMean = totalSensorMean; }

private:
  uint16_t _analogData[sensorCount];
  uint16_t _voltageData[sensorCount];
  uint16_t _resistanceData[sensorCount];
  uint16_t _conductanceData[sensorCount];
  float _forceData[sensorCount];
  uint16_t _analogOffset[sensorCount];
  int _totalSensorMean;
};

#endif /* _FORCESENSOR_H_ */
