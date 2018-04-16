#include "FileManager.h"
#include <iostream>
#include <fstream>

const std::string VALUE_SEPARATOR = ",";
const std::string SENSOR_SEPARATOR = "; ";
const std::string OFFSETS_FILENAME = "offsets.txt";
const std::string GYRO_FIELDNAME = "Gyro: ";
const std::string ACCEL_FIELDNAME = "Accel: ";
enum axis { x, y, z };

int * FileManager::GetFixedImuAccelOffsets()
{
    if (_fixedImuAccelerationOffsets[axis::x] != 0 || _fixedImuAccelerationOffsets[axis::y] != 0 || _fixedImuAccelerationOffsets[axis::z] != 0)
    {
	    return _fixedImuAccelerationOffsets;
    }

    return NULL;
}

int * FileManager::GetFixedImuGyroOffsets()
{
	if (_fixedImuGyroOffsets[axis::x] != 0 || _fixedImuGyroOffsets[axis::y] != 0 || _fixedImuGyroOffsets[axis::z] != 0)
	{
		return _fixedImuGyroOffsets;
	}

	return NULL;
}

int * FileManager::GetMobileImuAccelOffsets()
{
	if (_mobileImuAccelerationOffsets[axis::x] != 0 || _mobileImuAccelerationOffsets[axis::y] != 0 || _mobileImuAccelerationOffsets[axis::z] != 0)
	{
		return _mobileImuAccelerationOffsets;
	}

	return NULL;
}

int * FileManager::GetMobileImuGyroOffsets()
{
	if (_mobileImuGyroOffsets[axis::x] != 0 || _mobileImuGyroOffsets[axis::y] != 0 || _mobileImuGyroOffsets[axis::z] != 0)
	{
		return _mobileImuGyroOffsets;
	}

	return NULL;
}

void FileManager::WriteCalibrationOffsetsToFile(int * accelerationOffsets, int * gyroOffsets, std::string imuName)
{
	std::ofstream file;

	file.open(OFFSETS_FILENAME, std::ofstream::out | std::ofstream::app);

	if (!file.is_open())
	{
		return;
	}

	file << imuName << std::endl;
	file << ACCEL_FIELDNAME << accelerationOffsets[axis::x] << VALUE_SEPARATOR << accelerationOffsets[axis::y] << VALUE_SEPARATOR << accelerationOffsets[axis::z] << SENSOR_SEPARATOR;
	file << GYRO_FIELDNAME << gyroOffsets[axis::x] << VALUE_SEPARATOR << gyroOffsets[axis::y] << VALUE_SEPARATOR << gyroOffsets[axis::z] << std::endl;
	file.close();
}

void FileManager::ReadCalibrationOffsetsFromFile(std::string fixedImuName, std::string mobileImuName)
{
	std::string imuName;
	std::string fixedImuLine;
	std::string mobileImuLine;

	std::ifstream file(OFFSETS_FILENAME);

	if (!file.is_open())
	{
		return;
	}

	while (getline(file, imuName))
	{
		if (imuName == fixedImuName)
		{
			getline(file, fixedImuLine);
		}
		else if (imuName == mobileImuName)
		{
			getline(file, mobileImuLine);
		}
	}

	file.close();

	SetFixedImuOffsets(fixedImuLine);
	SetMobileImuOffsets(mobileImuLine);
}

void FileManager::SetOffsetsFromLine(std::string line, int * offsets, std::string offsetsLine)
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
		offsets[axis::x] = 0;
		offsets[axis::y] = 0;
		offsets[axis::z] = 0;
		return;
	}

	offsets[i] = value;
}

void FileManager::SetFixedImuOffsets(std::string line)
{
	std::string accelOffsetsLine = line.substr(0, line.find(SENSOR_SEPARATOR));
	accelOffsetsLine = accelOffsetsLine.substr(accelOffsetsLine.find(ACCEL_FIELDNAME) + ACCEL_FIELDNAME.length());

	std::string gyroOffsetsLine = line.substr(line.find(SENSOR_SEPARATOR) + SENSOR_SEPARATOR.length());
	gyroOffsetsLine = gyroOffsetsLine.substr(gyroOffsetsLine.find(GYRO_FIELDNAME) + GYRO_FIELDNAME.length());

	SetOffsetsFromLine(line, _fixedImuAccelerationOffsets, accelOffsetsLine);
	SetOffsetsFromLine(line, _fixedImuGyroOffsets, gyroOffsetsLine);
}

void FileManager::SetMobileImuOffsets(std::string line)
{
	std::string accelOffsetsLine = line.substr(0, line.find(SENSOR_SEPARATOR));
	accelOffsetsLine = accelOffsetsLine.substr(accelOffsetsLine.find(ACCEL_FIELDNAME) + ACCEL_FIELDNAME.length());

	std::string gyroOffsetsLine = line.substr(line.find(SENSOR_SEPARATOR) + SENSOR_SEPARATOR.length());
	gyroOffsetsLine = gyroOffsetsLine.substr(gyroOffsetsLine.find(GYRO_FIELDNAME) + GYRO_FIELDNAME.length());

	SetOffsetsFromLine(line, _mobileImuAccelerationOffsets, accelOffsetsLine);
	SetOffsetsFromLine(line, _mobileImuGyroOffsets, gyroOffsetsLine);
}
