#include "BackSeatAngleTracker.h"
#include "Utils.h"

#include <algorithm>
#include <math.h>
#include <string>
#include <unistd.h>

enum _axis { x, y, z };
const std::string fixedImuName = "fixedImu";
const std::string mobileImuName = "mobileImu";

BackSeatAngleTracker::BackSeatAngleTracker()
{
}

bool BackSeatAngleTracker::Initialize()
{
    _fileManager.ReadCalibrationOffsetsFromFile(fixedImuName, mobileImuName);
    return InitializeMobileImu() && InitializeFixedImu();
}

bool BackSeatAngleTracker::InitializeImu(MPU6050 &mpu, std::string name, int *accelerometerOffsets, int *gyroscopeOffsets)
{
    printf("MPU6050 %s initializing ... ", name.c_str());
    fflush(stdout);

    if (!mpu.TestConnection())
    {
        printf("FAIL\n");
        return false;
    }

    mpu.Initialize();

    ResetIMUOffsets(mpu);

    if (accelerometerOffsets == NULL || gyroscopeOffsets == NULL)
    {
        Calibrate(mpu, name);
    }
    else
    {
        std::copy(accelerometerOffsets, accelerometerOffsets + NUMBER_OF_AXIS, std::begin(_accelerometerOffsets));
        std::copy(gyroscopeOffsets, gyroscopeOffsets + NUMBER_OF_AXIS, std::begin(_gyroscopeOffsets));
    }

    SetImuOffsets(mpu);
    printf("SUCCESS\n");
    return true;
}

bool BackSeatAngleTracker::InitializeFixedImu()
{
    int *accelerometerOffsets = _fileManager.GetFixedImuAccelOffsets();
    int *gyroscopeOffsets = _fileManager.GetFixedImuGyroOffsets();

    return InitializeImu(_fixedImu, fixedImuName, accelerometerOffsets, gyroscopeOffsets);
}

bool BackSeatAngleTracker::InitializeMobileImu()
{
    int *accelerometerOffsets = _fileManager.GetMobileImuAccelOffsets();
    int *gyroscopeOffsets = _fileManager.GetMobileImuGyroOffsets();

    return InitializeImu(_mobileImu, mobileImuName, accelerometerOffsets, gyroscopeOffsets);
}

void BackSeatAngleTracker::ResetIMUOffsets(MPU6050 &mpu)
{
    ResetIMUAccelOffsets(mpu);
    ResetIMUGyroOffsets(mpu);
}

void BackSeatAngleTracker::ResetIMUAccelOffsets(MPU6050 &mpu)
{
    mpu.SetXAccelOffset(0);
    mpu.SetYAccelOffset(0);
    mpu.SetZAccelOffset(0);
}

void BackSeatAngleTracker::ResetIMUGyroOffsets(MPU6050 &mpu)
{
    mpu.SetXGyroOffset(0);
    mpu.SetYGyroOffset(0);
    mpu.SetZGyroOffset(0);
}

void BackSeatAngleTracker::SetImuOffsets(MPU6050 &mpu)
{
    SetImuAccelOffsets(mpu);
    SetImuGyroOffsets(mpu);
}

void BackSeatAngleTracker::SetImuAccelOffsets(MPU6050 &mpu)
{
    mpu.SetXAccelOffset(_accelerometerOffsets[_axis::x]);
    mpu.SetYAccelOffset(_accelerometerOffsets[_axis::y]);
    mpu.SetZAccelOffset(_accelerometerOffsets[_axis::z]);
}

void BackSeatAngleTracker::SetImuGyroOffsets(MPU6050 &mpu)
{
    mpu.SetXGyroOffset(_gyroscopeOffsets[_axis::x]);
    mpu.SetYGyroOffset(_gyroscopeOffsets[_axis::y]);
    mpu.SetZGyroOffset(_gyroscopeOffsets[_axis::z]);
}

void BackSeatAngleTracker::GetGyroscopeMeans(MPU6050 &mpu, int gyroscopeMeans[])
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
            gyroscopeMeans[_axis::x] += gx;
            gyroscopeMeans[_axis::y] += gy;
            gyroscopeMeans[_axis::z] += gz;
        }

        sleep_for_microseconds(timeBetweenMeasures);
    }

    gyroscopeMeans[_axis::x] /= BUFFER_SIZE;
    gyroscopeMeans[_axis::y] /= BUFFER_SIZE;
    gyroscopeMeans[_axis::z] /= BUFFER_SIZE;
}

void BackSeatAngleTracker::GetAccelerometerMeans(MPU6050 &mpu, int accelerationBuffer[])
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
            accelerationBuffer[_axis::x] += ax;
            accelerationBuffer[_axis::y] += ay;
            accelerationBuffer[_axis::z] += az;
        }

        sleep_for_microseconds(timeBetweenMeasures);
    }

    accelerationBuffer[_axis::x] /= BUFFER_SIZE;
    accelerationBuffer[_axis::y] /= BUFFER_SIZE;
    accelerationBuffer[_axis::z] /= BUFFER_SIZE;
}

void BackSeatAngleTracker::CalibrateAccelerometer(MPU6050 &mpu)
{
    uint8_t ready = 0;
    int accelerometerMeans[NUMBER_OF_AXIS] = {0, 0, 0};
    GetAccelerometerMeans(mpu, accelerometerMeans);

    _accelerometerOffsets[_axis::x] = (_calibrationArray[_axis::x] - accelerometerMeans[_axis::x]) / 8;
    _accelerometerOffsets[_axis::y] = (_calibrationArray[_axis::y] - accelerometerMeans[_axis::y]) / 8;
    _accelerometerOffsets[_axis::z] = (_calibrationArray[_axis::z] - accelerometerMeans[_axis::z]) / 8;

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

void BackSeatAngleTracker::CalibrateGyroscope(MPU6050 &mpu)
{
    uint8_t ready = 0;
    int gyroscopeMeans[NUMBER_OF_AXIS] = {0, 0, 0};
    GetGyroscopeMeans(mpu, gyroscopeMeans);

    _gyroscopeOffsets[_axis::x] = -gyroscopeMeans[_axis::x] / 4;
    _gyroscopeOffsets[_axis::y] = -gyroscopeMeans[_axis::y] / 4;
    _gyroscopeOffsets[_axis::z] = -gyroscopeMeans[_axis::z] / 4;

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

void BackSeatAngleTracker::Calibrate(MPU6050 &mpu, std::string name)
{
    CalibrateAccelerometer(mpu);
    CalibrateGyroscope(mpu);

    _fileManager.WriteCalibrationOffsetsToFile(_accelerometerOffsets, _gyroscopeOffsets, name);
}

void BackSeatAngleTracker::Calibrate()
{
    // Calibrate fixed IMU:
    ResetIMUOffsets(_fixedImu);
    Calibrate(_fixedImu, fixedImuName);
    SetImuOffsets(_fixedImu);

    _fileManager.WriteCalibrationOffsetsToFile(_accelerometerOffsets, _gyroscopeOffsets, fixedImuName);

    // Calibrate mobile IMU:
    ResetIMUOffsets(_mobileImu);
    Calibrate(_mobileImu, mobileImuName);
    SetImuOffsets(_mobileImu);

    _fileManager.WriteCalibrationOffsetsToFile(_accelerometerOffsets, _gyroscopeOffsets, mobileImuName);
}

void BackSeatAngleTracker::GetAcceleration(MPU6050 &mpu, double *acceleration)
{
    int16_t ax, ay, az;
    mpu.GetAcceleration(&ax, &ay, &az);

    // TODO: Add low-pass filter

    acceleration[_axis::x] = double(ax) * 2 / 32768.0f;
    acceleration[_axis::y] = double(ay) * 2 / 32768.0f;
    acceleration[_axis::z] = double(az) * 2 / 32768.0f;
}

double BackSeatAngleTracker::GetPitch(double acceleration[])
{
    return atan2(acceleration[_axis::x], sqrt(acceleration[_axis::y] * acceleration[_axis::y] + acceleration[_axis::z] * acceleration[_axis::z])) * radiansToDegrees;
}

int BackSeatAngleTracker::GetBackSeatAngle()
{
    double fixedImuAccelerations[NUMBER_OF_AXIS] = {0, 0, 0};
    double mobileImuAccelerations[NUMBER_OF_AXIS] = {0, 0, 0};

    GetAcceleration(_fixedImu, fixedImuAccelerations);
    GetAcceleration(_mobileImu, mobileImuAccelerations);

    double fixedPitch = GetPitch(fixedImuAccelerations);
    double mobilePitch = GetPitch(mobileImuAccelerations);

    return abs(int(fixedPitch - mobilePitch));
}
