#include "MobileImu.h"
#include "Utils.h"
#include <string>
#include <unistd.h>

MobileImu::MobileImu()
{
    _imuName = MOBILE_IMU_NAME;
    _imu = {0x68};
}

bool MobileImu::isSetup()
{
    int *accelerometerOffsets = _fileManager.GetMobileImuAccelOffsets();
    int *gyroscopeOffsets = _fileManager.GetMobileImuGyroOffsets();

    printf("MPU6050 %s initializing ... ", MOBILE_IMU_NAME.c_str());
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
        Calibrate(_imu, MOBILE_IMU_NAME);
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
