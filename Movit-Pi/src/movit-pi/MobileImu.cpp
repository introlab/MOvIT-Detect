#include "MobileImu.h"
#include "Utils.h"
#include <string>
#include <unistd.h>

MobileImu::MobileImu()
{
    _imuName = MOBILE_IMU_NAME;
    _imu = {0x69};
}
