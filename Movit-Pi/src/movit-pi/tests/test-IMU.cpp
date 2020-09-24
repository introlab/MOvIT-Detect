#include "MPU6050.h"
#include <iostream>

using namespace std;

void read_imu(MPU6050 &imu)
{
    short ax,ay,az,gx,gy,gz;
    imu.GetMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    double acc_x = static_cast<double>(ax) / 16384.0;
    double acc_y = static_cast<double>(ay) / 16384.0;
    double acc_z = static_cast<double>(az) / 16384.0;
    double gyro_x = static_cast<double>(gx) * 131.0;
    double gyro_y = static_cast<double>(gy) * 131.0;
    double gyro_z = static_cast<double>(gz) * 131.0;

    cout << acc_x << ", " << acc_y << ", " << acc_z << ", " << gyro_x << ", " << gyro_y << ", " << gyro_z << endl;

}



int main(int argc, char* argv[])
{
    MPU6050 fixed(0x68);
    MPU6050 mobile(0x69);

    fixed.Initialize();
    mobile.Initialize();

    while(1)
    {
        read_imu(fixed);
        read_imu(mobile);
        sleep(1);
    }

    return 0;
}
