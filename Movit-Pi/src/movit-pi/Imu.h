#ifndef IMU_H
#define IMU_H

#include <string>

#include "MPU6050.h"
#include "Utils.h"
#include "DataType.h"
#include "Sensor.h"

#define ACCELEROMETER_DEADZONE 8 // Accelerometer error allowed, make it lower to get more precision, but sketch may not converge (default: 8)
#define BUFFER_SIZE 1000         // Amount of readings used to average, make it higher to get more precision but sketch will be slower (default: 1000)
#define GYROSCOPE_DEADZONE 1     // Gyroscope error allowed, make it lower to get more precision, but sketch may not converge (default: 1)
#define GRAVITY 9.80665
#define LSB_SENSITIVITY -16384

class Imu : public Sensor
{
  public:
    Imu();
    bool Initialize();
    bool IsConnected();
    void CalibrateAndSetOffsets();
    void GetAccelerations(double *accelerations);
    double GetPitch();
    double GetRoll();

    static bool IsImuOffsetValid(imu_offset_t offset);

    void SetOffset(imu_offset_t offsets);
    imu_offset_t GetOffset() { return _offsets; }

  protected:
    std::string _imuName;
    MPU6050 _imu;
    int _calibrationArray[NUMBER_OF_AXIS] = {LSB_SENSITIVITY, 0, 0};

    imu_offset_t _offsets;

    void Calibrate(MPU6050 &mpu, std::string name);
    void CalibrateAccelerometer(MPU6050 &mpu);
    void CalibrateGyroscope(MPU6050 &mpu);

    void SetImuOffsets(MPU6050 &mpu);
    void SetImuAccelOffsets(MPU6050 &mpu);
    void SetImuGyroOffsets(MPU6050 &mpu);
    void ResetIMUOffsets(MPU6050 &mpu);
    void ResetIMUAccelOffsets(MPU6050 &mpu);
    void ResetIMUGyroOffsets(MPU6050 &mpu);

    void GetAccelerometerMeans(MPU6050 &mpu, int accbuff[]);
    void GetGyroscopeMeans(MPU6050 &mpu, int gyrobuff[]);
};

#endif // IMU_H
