#include "Imu.h"
#include "Utils.h"

#include <algorithm>
#include <math.h>
#include <unistd.h>

Imu::Imu()
{
}

bool Imu::isInitialized()
{
    _fileManager.ReadCalibrationOffsetsFromFile(_imuName);
    return isSetup();
}

void Imu::CalibrateAndSetOffsets()
{
    ResetIMUOffsets(_imu);
    Calibrate(_imu, _imuName);
    SetImuOffsets(_imu);

    _fileManager.WriteCalibrationOffsetsToFile(_accelerometerOffsets, _gyroscopeOffsets, _imuName);
}

void Imu::ResetIMUOffsets(MPU6050 &mpu)
{
    ResetIMUAccelOffsets(mpu);
    ResetIMUGyroOffsets(mpu);
}

void Imu::ResetIMUAccelOffsets(MPU6050 &mpu)
{
    mpu.SetXAccelOffset(0);
    mpu.SetYAccelOffset(0);
    mpu.SetZAccelOffset(0);
}

void Imu::ResetIMUGyroOffsets(MPU6050 &mpu)
{
    mpu.SetXGyroOffset(0);
    mpu.SetYGyroOffset(0);
    mpu.SetZGyroOffset(0);
}

void Imu::SetImuOffsets(MPU6050 &mpu)
{
    SetImuAccelOffsets(mpu);
    SetImuGyroOffsets(mpu);
}

void Imu::SetImuAccelOffsets(MPU6050 &mpu)
{
    mpu.SetXAccelOffset(_accelerometerOffsets[AXIS::x]);
    mpu.SetYAccelOffset(_accelerometerOffsets[AXIS::y]);
    mpu.SetZAccelOffset(_accelerometerOffsets[AXIS::z]);
}

void Imu::SetImuGyroOffsets(MPU6050 &mpu)
{
    mpu.SetXGyroOffset(_gyroscopeOffsets[AXIS::x]);
    mpu.SetYGyroOffset(_gyroscopeOffsets[AXIS::y]);
    mpu.SetZGyroOffset(_gyroscopeOffsets[AXIS::z]);
}

void Imu::GetGyroscopeMeans(MPU6050 &mpu, int gyroscopeMeans[])
{
    const int numberOfDiscardedMeasures = 100;
    const int timeBetweenMeasures = 2000;

    uint16_t i = 0;
    int16_t gx, gy, gz;

    while (i < (BUFFER_SIZE + numberOfDiscardedMeasures))
    {
        mpu.GetRotation(&gx, &gy, &gz);

        if (i++ > numberOfDiscardedMeasures)
        {
            gyroscopeMeans[AXIS::x] += gx;
            gyroscopeMeans[AXIS::y] += gy;
            gyroscopeMeans[AXIS::z] += gz;
        }

        sleep_for_microseconds(timeBetweenMeasures);
    }

    gyroscopeMeans[AXIS::x] /= BUFFER_SIZE;
    gyroscopeMeans[AXIS::y] /= BUFFER_SIZE;
    gyroscopeMeans[AXIS::z] /= BUFFER_SIZE;
}

void Imu::GetAccelerometerMeans(MPU6050 &mpu, int accelerationBuffer[])
{
    const int numberOfDiscardedMeasures = 100;
    const int timeBetweenMeasures = 2000;

    uint16_t i = 0;
    int16_t ax, ay, az;

    while (i < (BUFFER_SIZE + numberOfDiscardedMeasures))
    {
        mpu.GetAcceleration(&ax, &ay, &az);

        if (i++ > numberOfDiscardedMeasures)
        {
            accelerationBuffer[AXIS::x] += ax;
            accelerationBuffer[AXIS::y] += ay;
            accelerationBuffer[AXIS::z] += az;
        }

        sleep_for_microseconds(timeBetweenMeasures);
    }

    accelerationBuffer[AXIS::x] /= BUFFER_SIZE;
    accelerationBuffer[AXIS::y] /= BUFFER_SIZE;
    accelerationBuffer[AXIS::z] /= BUFFER_SIZE;
}

void Imu::CalibrateAccelerometer(MPU6050 &mpu)
{
    uint8_t ready = 0;
    int accelerometerMeans[NUMBER_OF_AXIS] = {0, 0, 0};
    GetAccelerometerMeans(mpu, accelerometerMeans);

    _accelerometerOffsets[AXIS::x] = (_calibrationArray[AXIS::x] - accelerometerMeans[AXIS::x]) / 8;
    _accelerometerOffsets[AXIS::y] = (_calibrationArray[AXIS::y] - accelerometerMeans[AXIS::y]) / 8;
    _accelerometerOffsets[AXIS::z] = (_calibrationArray[AXIS::z] - accelerometerMeans[AXIS::z]) / 8;

    while (ready < NUMBER_OF_AXIS)
    {
        ready = 0;
        SetImuAccelOffsets(mpu);
        GetAccelerometerMeans(mpu, accelerometerMeans);

        for (uint8_t i = 0; i < NUMBER_OF_AXIS; i++)
        {
            printf("%i\n", abs(_calibrationArray[i] - accelerometerMeans[i]));

            if (abs(_calibrationArray[i] - accelerometerMeans[i]) <= ACCELEROMETER_DEADZONE)
            {
                ready++;
            }
            else
            {
                _accelerometerOffsets[i] = _accelerometerOffsets[i] + (_calibrationArray[i] - accelerometerMeans[i]) / ACCELEROMETER_DEADZONE;
            }
        }
    }
}

void Imu::CalibrateGyroscope(MPU6050 &mpu)
{
    uint8_t ready = 0;
    int gyroscopeMeans[NUMBER_OF_AXIS] = {0, 0, 0};
    GetGyroscopeMeans(mpu, gyroscopeMeans);

    _gyroscopeOffsets[AXIS::x] = -gyroscopeMeans[AXIS::x] / 4;
    _gyroscopeOffsets[AXIS::y] = -gyroscopeMeans[AXIS::y] / 4;
    _gyroscopeOffsets[AXIS::z] = -gyroscopeMeans[AXIS::z] / 4;

    while (ready < NUMBER_OF_AXIS)
    {
        ready = 0;
        SetImuGyroOffsets(mpu);
        GetGyroscopeMeans(mpu, gyroscopeMeans);

        for (uint8_t i = 0; i < NUMBER_OF_AXIS; i++)
        {
            if (abs(gyroscopeMeans[i]) <= GYROSCOPE_DEADZONE)
            {
                ready++;
            }
            else
            {
                _gyroscopeOffsets[i] = _gyroscopeOffsets[i] - gyroscopeMeans[i] / (GYROSCOPE_DEADZONE + 1);
            }
        }
    }
}

void Imu::Calibrate(MPU6050 &mpu, std::string name)
{
    CalibrateAccelerometer(mpu);
    CalibrateGyroscope(mpu);

    _fileManager.WriteCalibrationOffsetsToFile(_accelerometerOffsets, _gyroscopeOffsets, name);
}

void Imu::GetAccelerations(double *accelerations)
{
    int16_t ax, ay, az;
    
    _imu.GetAcceleration(&ax, &ay, &az);

    // TODO: Add low-pass filter

    accelerations[AXIS::x] = double(ax) * 2 / 32768.0f;
    accelerations[AXIS::y] = double(ay) * 2 / 32768.0f;
    accelerations[AXIS::z] = double(az) * 2 / 32768.0f;
}
