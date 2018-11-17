#ifndef FORCE_SENSOR_H
#define FORCE_SENSOR_H

#include "MAX11611.h"
#include "Utils.h"

class ForceSensor
{
  public:
    ForceSensor();
    ~ForceSensor();

    void CalibrateForceSensor(MAX11611 &max11611, uint16_t *max11611Data, uint8_t maxIteration);
    bool IsUserDetected();

    pressure_mat_offset_t GetOffsets();
    uint16_t GetAnalogData(uint8_t index);

    void SetOffsets(pressure_mat_offset_t offset);
    void SetAnalogData(uint8_t index, uint16_t analogdata) { _analogData[index] = analogdata; }

  private:
    uint16_t _analogData[PRESSURE_SENSOR_COUNT];
    uint16_t _analogOffset[PRESSURE_SENSOR_COUNT];
    uint32_t _totalSensorMean;
    float _detectionThreshold;
};

#endif // FORCE_SENSOR_H
