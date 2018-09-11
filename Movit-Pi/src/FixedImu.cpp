#include "FixedImu.h"
#include "Utils.h"
#include <string>
#include <unistd.h>

FixedImu::FixedImu()
{
    _imuName = FIXED_IMU_NAME;
    _imu = {0x69};
}

bool FixedImu::isSetup()
{
    int *accelerometerOffsets = _fileManager.GetFixedImuAccelOffsets();
    int *gyroscopeOffsets = _fileManager.GetFixedImuGyroOffsets();

    printf("MPU6050 %s initializing ... ", FIXED_IMU_NAME.c_str());
    fflush(stdout);

    if (!_imu.TestConnection())
    {
        printf("FAIL\n");
        return false;
    }

    _imu.Initialize();

    ResetIMUOffsets(_imu);

    if (accelerometerOffsets == NULL || gyroscopeOffsets == NULL)
    {
        Calibrate(_imu, FIXED_IMU_NAME);
    }
    else
    {
        std::copy(accelerometerOffsets, accelerometerOffsets + NUMBER_OF_AXIS, std::begin(_accelerometerOffsets));
        std::copy(gyroscopeOffsets, gyroscopeOffsets + NUMBER_OF_AXIS, std::begin(_gyroscopeOffsets));
    }

    SetImuOffsets(_imu);
    printf("SUCCESS\n");
    return true;
}

double FixedImu::GetXAcceleration()
{
    int16_t ax, ay, az;

    _imu.GetAcceleration(&ax, &ay, &az);

    return double(ax) * 2 / 32768.0f;
}
