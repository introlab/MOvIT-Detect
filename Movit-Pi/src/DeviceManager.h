#ifndef DEVICE_MANAGER
#define DEVICE_MANAGER

#include "alarm.h"
#include "forcePlate.h"
#include "forceSensor.h"
#include "MAX11611.h" //10-Bit ADC librairy
#include "accel_module.h"
#include "DateTimeRTC.h"
#include <string>

//Center of pressure coordinate
struct Coord_t
{
    uint16_t x;
    uint16_t y;
};

class DeviceManager
{
  public:
    void InitializeDevices();

    // Called periodicaly to update all the data
    void update();

    Alarm *getAlarm() { return &_alarm; }

    bool isSomeoneThere() { return _isSomeoneThere; }
    Coord_t getCenterOfPressure() { return _COPCoord; }
    int GetBackSeatAngle() { return _backSeatAngle; }
    std::string getDateTime() { return _currentDateTimeStr; }

    bool testDevices();

    // Singleton
    static DeviceManager *getInstance()
    {
        static DeviceManager instance;
        return &instance;
    }

  private:
    //Singleton
    DeviceManager();
    DeviceManager(DeviceManager const &);  // Don't Implement.
    void operator=(DeviceManager const &); // Don't implement.

    void updateForcePlateData();

    bool _isSomeoneThere = false;
    Coord_t _COPCoord;

    unsigned char _dateTimeRaw[DATE_TIME_SIZE] = {0x45, 0x59, 0x12, 0x00, 0x11, 0x03, 0x18}; //En BCD {seconds, minutes, hours, day, date, month, year}
    std::string _currentDateTimeStr = "";

    DateTimeRTC *_datetimeRTC;

    Alarm _alarm;

    BackSeatAngleTracker _imu;
    int _backSeatAngle = 0;

    uint16_t max11611Data[9];
    forceSensor sensorMatrix;
    forcePlate globalForcePlate;
    MAX11611 max11611;
};

#endif //DEVICE_MANAGER
