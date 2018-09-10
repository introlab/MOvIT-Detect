#ifndef FORCE_SENSOR_H
#define FORCE_SENSOR_H

#include "MAX11611.h"

class ForceSensor
{
  public:
    ForceSensor();
    ~ForceSensor();

    void CalibrateForceSensor(uint16_t *max11611Data, MAX11611 &max11611);
    void GetForceSensorData();
    bool IsUserDetected();
    int *DetectRelativePressure();

    uint16_t GetAnalogData(uint8_t index) { return _analogData[index]; }
    uint16_t GetVoltageData(uint8_t index) { return _voltageData[index]; };
    uint16_t GetAnalogOffset(uint8_t index) { return _analogOffset[index]; };
    uint32_t GetConductanceData(uint8_t index) { return _conductanceData[index]; };
    uint32_t GetResistanceData(uint8_t index) { return _resistanceData[index]; };
    uint32_t GetTotalSensorMean() { return _totalSensorMean; };
    float GetForceData(uint8_t index) { return _forceData[index]; };
    float GetDetectionThreshold() { return _detectionThreshold; };

    void SetAnalogData(uint16_t analogdata, uint8_t index) { _analogData[index] = analogdata; }
    void SetVoltageData(uint16_t voltageData, uint8_t index) { _voltageData[index] = voltageData; }
    void SetAnalogOffset(uint16_t analogOffset, uint8_t index) { _analogOffset[index] = analogOffset; }
    void SetResistanceData(uint32_t resistanceData, uint8_t index) { _resistanceData[index] = resistanceData; }
    void SetConductanceData(uint32_t conductanceData, uint8_t index) { _conductanceData[index] = conductanceData; }
    void SetTotalSensorMean(uint32_t totalSensorMean) { _totalSensorMean = totalSensorMean; }
    void SetForceData(float forceData, uint8_t index) { _forceData[index] = forceData; }
    void SetDetectionThreshold(float detectionThreshold) { _detectionThreshold = detectionThreshold; }

    //Constants - physical montage values
    static const uint8_t _sensorCount = 9; //Total number of sensor in the matrix

  private:
    uint16_t _analogData[_sensorCount];
    uint16_t _voltageData[_sensorCount];
    uint16_t _analogOffset[_sensorCount];
    uint32_t _conductanceData[_sensorCount];
    uint32_t _resistanceData[_sensorCount];
    uint32_t _totalSensorMean;
    float _forceData[_sensorCount];
    float _detectionThreshold;
};

#endif // FORCE_SENSOR_H
