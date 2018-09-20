#include "FileManager.h"
#include "Utils.h"
#include <iostream>
#include <fstream>

const std::string VALUE_SEPARATOR = ",";
const std::string SENSOR_SEPARATOR = "; ";
const std::string OFFSETS_FILENAME = "offsets.txt";
const std::string GYRO_FIELDNAME = "Gyro: ";
const std::string ACCEL_FIELDNAME = "Accel: ";

int *FileManager::GetFixedImuAccelOffsets()
{
    if (_fixedImuAccelerationOffsets[AXIS::x] != 0 || _fixedImuAccelerationOffsets[AXIS::y] != 0 || _fixedImuAccelerationOffsets[AXIS::z] != 0)
    {
        return _fixedImuAccelerationOffsets;
    }

    return NULL;
}

int *FileManager::GetFixedImuGyroOffsets()
{
    if (_fixedImuGyroOffsets[AXIS::x] != 0 || _fixedImuGyroOffsets[AXIS::y] != 0 || _fixedImuGyroOffsets[AXIS::z] != 0)
    {
        return _fixedImuGyroOffsets;
    }

    return NULL;
}

int *FileManager::GetMobileImuAccelOffsets()
{
    if (_mobileImuAccelerationOffsets[AXIS::x] != 0 || _mobileImuAccelerationOffsets[AXIS::y] != 0 || _mobileImuAccelerationOffsets[AXIS::z] != 0)
    {
        return _mobileImuAccelerationOffsets;
    }

    return NULL;
}

int *FileManager::GetMobileImuGyroOffsets()
{
    if (_mobileImuGyroOffsets[AXIS::x] != 0 || _mobileImuGyroOffsets[AXIS::y] != 0 || _mobileImuGyroOffsets[AXIS::z] != 0)
    {
        return _mobileImuGyroOffsets;
    }

    return NULL;
}

void FileManager::DeleteExistingOffsets(std::string imuName)
{
    std::string line;
    std::fstream fs;
    std::string tempString;

    fs.open(OFFSETS_FILENAME);

    if (!fs.is_open())
    {
        return;
    }

    while (getline(fs, line))
    {
        if (line == imuName)
        {
            getline(fs, line);
        }
        else
        {
            tempString += line.c_str();
            tempString += "\n";
        }
    }

    fs.close();
    fs.open(OFFSETS_FILENAME,  std::ofstream::out | std::ofstream::trunc);

    if (!fs.is_open())
    {
        printf("Error opening the file...\n");
        return;
    }

    fs << tempString;
    fs.close();
}

void FileManager::WriteCalibrationOffsetsToFile(int *accelerationOffsets, int *gyroOffsets, std::string imuName)
{
    DeleteExistingOffsets(imuName);

    std::ofstream fileout;

    fileout.open(OFFSETS_FILENAME, std::ofstream::out | std::ofstream::app);

    if (!fileout.is_open())
    {
        printf("Error opening the file...\n");
        return;
    }

    fileout << imuName << std::endl;
    fileout << ACCEL_FIELDNAME << accelerationOffsets[AXIS::x] << VALUE_SEPARATOR << accelerationOffsets[AXIS::y] << VALUE_SEPARATOR << accelerationOffsets[AXIS::z] << SENSOR_SEPARATOR;
    fileout << GYRO_FIELDNAME << gyroOffsets[AXIS::x] << VALUE_SEPARATOR << gyroOffsets[AXIS::y] << VALUE_SEPARATOR << gyroOffsets[AXIS::z] << std::endl;
    fileout.close();
}

void FileManager::ReadCalibrationOffsetsFromFile(std::string imuName)
{
    std::string name;
    std::string line;

    std::ifstream file(OFFSETS_FILENAME);

    if (!file.is_open())
    {
        printf("Error opening the file...\n");
        return;
    }

    while (getline(file, name))
    {
        if (name == imuName)
        {
            getline(file, line);
            break;
        }
    }

    file.close();

    if (line.length() == 0)
    {
        return;
    }

    if (imuName == FIXED_IMU_NAME)
    {
        SetImuOffsets(line, _fixedImuAccelerationOffsets, _fixedImuGyroOffsets);
    }
    else
    {
        SetImuOffsets(line, _mobileImuAccelerationOffsets, _mobileImuGyroOffsets);
    }
}

void FileManager::SetOffsetsFromLine(std::string line, int *offsets, std::string offsetsLine)
{
    const int numberOfSeparators = 2;
    uint8_t i = 0;
    size_t pos = 0;

    while ((pos = offsetsLine.find(VALUE_SEPARATOR)) != std::string::npos)
    {
        int value = atoi(offsetsLine.substr(0, pos).c_str());

        if (value == 0)
        {
            break;
        }

        offsets[i++] = value;
        offsetsLine.erase(0, pos + VALUE_SEPARATOR.length());
    }

    int value = atoi(offsetsLine.c_str());

    if (i != numberOfSeparators || value == 0)
    {
        offsets[AXIS::x] = 0;
        offsets[AXIS::y] = 0;
        offsets[AXIS::z] = 0;
        return;
    }

    offsets[i] = value;
}

void FileManager::SetImuOffsets(std::string line, int accelOffsets[], int gyroOffsets[])
{
    std::string accelOffsetsLine = line.substr(0, line.find(SENSOR_SEPARATOR));
    std::string gyroOffsetsLine = line.substr(line.find(SENSOR_SEPARATOR) + SENSOR_SEPARATOR.length());

    int accelOffsetsPosition = accelOffsetsLine.find(ACCEL_FIELDNAME);
    int gyroOffsetsPosition = gyroOffsetsLine.find(GYRO_FIELDNAME);

    if (accelOffsetsPosition != -1)
    {
        std::string line = accelOffsetsLine.substr(accelOffsetsPosition + ACCEL_FIELDNAME.length());
        SetOffsetsFromLine(line, accelOffsets, line);
    }
    if (gyroOffsetsPosition != -1)
    {
        std::string line = gyroOffsetsLine.substr(gyroOffsetsPosition + GYRO_FIELDNAME.length());
        SetOffsetsFromLine(line, gyroOffsets, line);
    }
}
