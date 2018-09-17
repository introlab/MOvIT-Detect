#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <string>

#define NUMBER_OF_AXIS 3

class FileManager
{
  public:
    int *GetFixedImuAccelOffsets();
    int *GetFixedImuGyroOffsets();
    int *GetMobileImuAccelOffsets();
    int *GetMobileImuGyroOffsets();

    void WriteCalibrationOffsetsToFile(int *accelerationOffsets, int *gyroOffsets, std::string type);
    void ReadCalibrationOffsetsFromFile(std::string imuName);

  private:
    int _fixedImuAccelerationOffsets[NUMBER_OF_AXIS];
    int _mobileImuAccelerationOffsets[NUMBER_OF_AXIS];
    int _fixedImuGyroOffsets[NUMBER_OF_AXIS];
    int _mobileImuGyroOffsets[NUMBER_OF_AXIS];

    void SetImuOffsets(std::string line, int accelOffsets[], int gyroOffsets[]);
    void SetOffsetsFromLine(std::string line, int *offsets, std::string offsetsLine);
};

#endif // FILE_MANAGER_H
