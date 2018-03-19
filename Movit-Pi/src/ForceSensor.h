#ifndef _FORCESENSOR_H_
#define _FORCESENSOR_H_

class forceSensor
{
  public:
    forceSensor();
    ~forceSensor();

    void CalibrateForceSensor(uint16_t* max11611Data, forceSensor &sensors);
    void GetForceSensorData(forceSensor &sensors);
    bool IsUserDetected(forceSensor &sensors);
    int *DetectRelativePressure(forceSensor &sensors);

    uint16_t GetAnalogData(int index) { return _analogData[index]; }
    long GetVoltageData(int index) { return _voltageData[index]; };
    long GetResistanceData(int index) { return _resistanceData[index]; };
    long GetConductanceData(int index) { return _conductanceData[index]; };
    float GetForceData(int index) { return _forceData[index]; };
    long GetAnalogOffset(int index) { return _analogOffset[index]; };
    int GetTotalSensorMean() { return _totalSensorMean; };
    double GetDetectionThreshold() { return _detectionThreshold; };

    void SetAnalogData(uint16_t analogdata, int index) { _analogData[index] = analogdata; }
    void SetVoltageData(uint16_t voltageData, int index) { _voltageData[index] = voltageData; }
    void SetResistanceData(uint16_t resistanceData, int index) { _resistanceData[index] = resistanceData; }
    void SetConductanceData(uint16_t conductanceData, int index) { _conductanceData[index] = conductanceData; }
    void SetForceData(float forceData, int index) { _forceData[index] = forceData; }
    void SetAnalogOffset(uint16_t analogOffset, int index) { _analogOffset[index] = analogOffset; }
    void SetTotalSensorMean(int totalSensorMean) { _totalSensorMean = totalSensorMean; }
    void SetDetectionThreshold(double detectionThreshold) { _detectionThreshold = detectionThreshold; }

    //Constants - physical montage values
    static const int sensorCount = 9;      //Total number of sensor in the matrix
    const float calibrationRatio = 0.75;

  private:
    uint16_t _analogData[sensorCount];
    uint16_t _voltageData[sensorCount];
    uint16_t _resistanceData[sensorCount];
    uint16_t _conductanceData[sensorCount];
    float _forceData[sensorCount];
    uint16_t _analogOffset[sensorCount];
    int _totalSensorMean;
    double _detectionThreshold;
};

#endif /* _FORCESENSOR_H_ */
