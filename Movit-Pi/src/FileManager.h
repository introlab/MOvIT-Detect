#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <string>

class FileManager
{
	public:
		int * GetFixedImuAccelOffsets();
		int * GetFixedImuGyroOffsets();
		int * GetMobileImuAccelOffsets();
		int * GetMobileImuGyroOffsets();

		void WriteImuCalibrationOffsetsToFile(int * accelerationOffsets, int * gyroOffsets, std::string type);
		void ReadImuCalibrationOffsetsFromFile(std::string fixedImuName, std::string mobileImuName);

	private:
		enum _axis { x, y, z };

		int _fixedImuAccelerationOffsets[3];
		int _mobileImuAccelerationOffsets[3];
		int _fixedImuGyroOffsets[3];
		int _mobileImuGyroOffsets[3];

		void SetFixedImuOffsets(std::string line);
		void SetMobileImuOffsets(std::string line);
		void SetOffsetsFromLine(std::string line, int * offsets, std::string offsetsLine);
};

#endif 
