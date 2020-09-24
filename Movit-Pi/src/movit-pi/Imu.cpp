#include "Imu.h"
#include "Utils.h"
#include "SysTime.h"

#include <algorithm>
#include <math.h>
#include <unistd.h>

Imu::Imu()
{
}

bool Imu::Initialize()
{
    //printf("MPU6050 %s initializing ... ", _imuName.c_str());
    //fflush(stdout);

    IsConnected();

    isInitialized = true;
    
    _imu.Initialize();



    if (IsConnected() == false || isInitialized == false)
    {
        isInitialized = false;
        return false;
    }

    return true;
}

bool Imu::IsConnected()
{
    bool state = _imu.TestConnection();
    
    if(state == false){
        isInitialized = false;
    }
    return state && isInitialized;
}

bool Imu::IsImuOffsetValid(imu_offset_t offset)
{
    return true;
    return offset.accelerometerOffsets[AXIS::x] != 0 && offset.accelerometerOffsets[AXIS::y] != 0 && offset.accelerometerOffsets[AXIS::z] != 0 &&
           offset.gyroscopeOffsets[AXIS::x] != 0 && offset.gyroscopeOffsets[AXIS::y] != 0 && offset.gyroscopeOffsets[AXIS::z] != 0;
}

void Imu::SetOffset(imu_offset_t offsets)
{
    _offsets = offsets;
    ResetIMUOffsets(_imu);
    SetImuOffsets(_imu);
}

void Imu::CalibrateAndSetOffsets()
{
    ResetIMUOffsets(_imu);
    Calibrate(_imu, _imuName);
    SetImuOffsets(_imu);
}

void Imu::Calibrate(MPU6050 &mpu, std::string name)
{
    CalibrateAccelerometer(mpu);
    CalibrateGyroscope(mpu);
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
    printf("offsets : %f %f %f\n", _offsets.accelerometerOffsets[AXIS::x], _offsets.accelerometerOffsets[AXIS::y], _offsets.accelerometerOffsets[AXIS::z]);
    mpu.SetXAccelOffset(_offsets.accelerometerOffsets[AXIS::x]);
    mpu.SetYAccelOffset(_offsets.accelerometerOffsets[AXIS::y]);
    mpu.SetZAccelOffset(_offsets.accelerometerOffsets[AXIS::z]);
}

void Imu::SetImuGyroOffsets(MPU6050 &mpu)
{
    mpu.SetXGyroOffset(_offsets.gyroscopeOffsets[AXIS::x]);
    mpu.SetYGyroOffset(_offsets.gyroscopeOffsets[AXIS::y]);
    mpu.SetZGyroOffset(_offsets.gyroscopeOffsets[AXIS::z]);
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

    _offsets.accelerometerOffsets[AXIS::x] = (_calibrationArray[AXIS::x] - accelerometerMeans[AXIS::x]) / 8;
    _offsets.accelerometerOffsets[AXIS::y] = (_calibrationArray[AXIS::y] - accelerometerMeans[AXIS::y]) / 8;
    _offsets.accelerometerOffsets[AXIS::z] = (_calibrationArray[AXIS::z] - accelerometerMeans[AXIS::z]) / 8;

    while (ready < NUMBER_OF_AXIS)
    {
        ready = 0;
        SetImuAccelOffsets(mpu);
        GetAccelerometerMeans(mpu, accelerometerMeans);

        for (uint8_t i = 0; i < NUMBER_OF_AXIS; i++)
        {
            printf(".");

            if (abs(_calibrationArray[i] - accelerometerMeans[i]) <= ACCELEROMETER_DEADZONE)
            {
                ready++;
            }
            else
            {
                _offsets.accelerometerOffsets[i] = _offsets.accelerometerOffsets[i] + (_calibrationArray[i] - accelerometerMeans[i]) / ACCELEROMETER_DEADZONE;
            }
        }
    }
}

void Imu::CalibrateGyroscope(MPU6050 &mpu)
{
    uint8_t ready = 0;
    int gyroscopeMeans[NUMBER_OF_AXIS] = {0, 0, 0};
    GetGyroscopeMeans(mpu, gyroscopeMeans);

    _offsets.gyroscopeOffsets[AXIS::x] = -gyroscopeMeans[AXIS::x] / 4;
    _offsets.gyroscopeOffsets[AXIS::y] = -gyroscopeMeans[AXIS::y] / 4;
    _offsets.gyroscopeOffsets[AXIS::z] = -gyroscopeMeans[AXIS::z] / 4;

    while (ready < NUMBER_OF_AXIS)
    {
        ready = 0;
        SetImuGyroOffsets(mpu);
        GetGyroscopeMeans(mpu, gyroscopeMeans);

        for (uint8_t i = 0; i < NUMBER_OF_AXIS; i++)
        {
            printf(".");
            if (abs(gyroscopeMeans[i]) <= GYROSCOPE_DEADZONE)
            {
                ready++;
            }
            else
            {
                _offsets.gyroscopeOffsets[i] = _offsets.gyroscopeOffsets[i] - gyroscopeMeans[i] / (GYROSCOPE_DEADZONE + 1);
            }
        }
    }
}

int8_t Imu::GetAccelerations(double *accelerations)
{
    int16_t ax, ay, az;

    int8_t count = _imu.GetAcceleration(&ax, &ay, &az);

    accelerations[AXIS::x] = static_cast<double>(ax) / (32768.0f / 2.0f);
    accelerations[AXIS::y] = static_cast<double>(ay) / (32768.0f / 2.0f);
    accelerations[AXIS::z] = static_cast<double>(az) / (32768.0f / 2.0f);
    return count;
}

int8_t Imu::GetMotion6(double *motion)
{
    
    int16_t ax, ay, az, gx, gy, gz;
    int8_t count = _imu.GetMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    motion[0] = static_cast<double>(ax) / (32768.0f / 2.0f);
    motion[1] = static_cast<double>(ay) / (32768.0f / 2.0f);
    motion[2] = static_cast<double>(az) / (32768.0f / 2.0f);
    motion[3] = static_cast<double>(gx) / (262.0f / 2.0f);
    motion[4] = static_cast<double>(gy) / (262.0f / 2.0f);
    motion[5] = static_cast<double>(gz) / (262.0f / 2.0f);

/*
    double test[3] = {0, 0, 0};
    _imu.GetRotation(&gx,&gy,&gz);
    test[0] = static_cast<double>(gx) / (262.0f / 2.0f);
    test[1] = static_cast<double>(gy) / (262.0f / 2.0f);
    test[2] = static_cast<double>(gz) / (262.0f / 2.0f);

    printf("motion %f %f %f \n", motion[3], motion[4], motion[5]);
    printf("test %f %f %f \n", test[0], test[1], test[2]);
*/
    return count;
}

bool Imu::dataReady() {
    return _imu.GetIntDataReadyStatus();
}

int8_t Imu::GetRotations(double *rotations)
{
    int16_t gx, gy, gz;
    int8_t count = _imu.GetRotation(&gx, &gy, &gz);
    rotations[AXIS::x] = static_cast<double>(gx) / (262.0f / 2.0f);
    rotations[AXIS::y] = static_cast<double>(gy) / (262.0f / 2.0f);
    rotations[AXIS::z] = static_cast<double>(gz) / (262.0f / 2.0f);
    return count;
}

double Imu::GetPitch()
{
    double accelerations[NUMBER_OF_AXIS] = {0, 0, 0};

    this->GetAccelerations(accelerations);

    return atan2(-1 * accelerations[AXIS::z], sqrt(accelerations[AXIS::x] * accelerations[AXIS::x] + accelerations[AXIS::y] * accelerations[AXIS::y])) * RADIANS_TO_DEGREES;
}

double Imu::GetRoll()
{
    double accelerations[NUMBER_OF_AXIS] = {0, 0, 0};

    this->GetAccelerations(accelerations);

    return atan2(accelerations[AXIS::x], accelerations[AXIS::y]) * RADIANS_TO_DEGREES + 90;
}

void Imu::ResetDevice()
{
    _imu.ResetDevice();
}
