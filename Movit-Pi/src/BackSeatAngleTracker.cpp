#include "BackSeatAngleTracker.h"
#include <algorithm> 
#include <unistd.h>
#include <math.h>
#include <string>

const std::string fixedImuName = "fixedImu";
const std::string mobileImuName = "mobileImu";

BackSeatAngleTracker::BackSeatAngleTracker()
{
}

bool BackSeatAngleTracker::Initialize()
{
	_fileManager.ReadImuCalibrationOffsetsFromFile(fixedImuName, mobileImuName);
	return InitializeMobileImu() && InitializeFixedImu();
}

bool BackSeatAngleTracker::InitializeFixedImu()
{
	printf("MPU6050 (imuFixe) initializing ... ");
	fflush(stdout);
	if (!_fixedImu.testConnection())
	{
		printf("FAIL\n");
		return false;
	}

	_fixedImu.initialize();

	int * accelOffsets = _fileManager.GetFixedImuAccelOffsets();
	int * gyroOffsets = _fileManager.GetFixedImuGyroOffsets();

	if (accelOffsets == NULL || gyroOffsets == NULL)
	{
		SetCalibrationArray(_axis::x);
		ResetIMUOffsets(_fixedImu);
		CalculateAccelerationsMean(_fixedImu);
		Calibrate(_fixedImu);
		CalculateAccelerationsMean(_fixedImu);

		_fileManager.WriteImuCalibrationOffsetsToFile(_accelerationOffsets, _gyroOffsets, fixedImuName);
	}
	else
	{
		std::copy(accelOffsets, accelOffsets + 3, std::begin(_accelerationOffsets));
		std::copy(gyroOffsets, gyroOffsets + 3, std::begin(_gyroOffsets));
	}

	SetImuOffsets(_fixedImu);
	return true;
}

bool BackSeatAngleTracker::InitializeMobileImu()
{
	printf("MPU6050 (imuMobile) initializing ... ");
	fflush(stdout);
	if (!_mobileImu.testConnection())
	{
		printf("FAIL\n");
		return false;
	}

	_mobileImu.initialize();

	int * accelOffsets = _fileManager.GetMobileImuAccelOffsets();
	int * gyroOffsets = _fileManager.GetMobileImuGyroOffsets();

	if (accelOffsets == NULL || gyroOffsets == NULL)
	{
		SetCalibrationArray(_axis::x);
		ResetIMUOffsets(_mobileImu);
		CalculateAccelerationsMean(_mobileImu);
		Calibrate(_mobileImu);
		CalculateAccelerationsMean(_mobileImu);

		_fileManager.WriteImuCalibrationOffsetsToFile(_accelerationOffsets, _gyroOffsets, mobileImuName);
	}
	else
	{
		std::copy(accelOffsets, accelOffsets + 3, std::begin(_accelerationOffsets));
		std::copy(gyroOffsets, gyroOffsets + 3, std::begin(_gyroOffsets));
	}

	SetImuOffsets(_mobileImu);
	return true;
}

void BackSeatAngleTracker::SetCalibrationArray(uint8_t axis)
{
	if (axis == _axis::x) // if X axis is pointing down
	{
		_calibrationArray[_axis::x] = -16384;
		_calibrationArray[_axis::y] = 0;
		_calibrationArray[_axis::z] = 0;
	}
	else if (axis == _axis::y) // if Y axis is pointing down
	{
		_calibrationArray[_axis::x] = 0;
		_calibrationArray[_axis::y] = -16384;
		_calibrationArray[_axis::z] = 0;
	}
	else if (axis == _axis::z) // if Z axis is pointing down
	{
		_calibrationArray[_axis::x] = 0;
		_calibrationArray[_axis::y] = 0;
		_calibrationArray[_axis::z] = -16384;
	}
}

void BackSeatAngleTracker::ResetIMUOffsets(MPU6050 &mpu)
{
	mpu.setXAccelOffset(0);
	mpu.setYAccelOffset(0);
	mpu.setZAccelOffset(0);

	mpu.setXGyroOffset(0);
	mpu.setYGyroOffset(0);
	mpu.setZGyroOffset(0);
}

void BackSeatAngleTracker::SetImuOffsets(MPU6050 &mpu)
{
	mpu.setXAccelOffset(_accelerationOffsets[_axis::x]);
	mpu.setYAccelOffset(_accelerationOffsets[_axis::y]);
	mpu.setZAccelOffset(_accelerationOffsets[_axis::z]);

	mpu.setXGyroOffset(_gyroOffsets[_axis::x]);
	mpu.setYGyroOffset(_gyroOffsets[_axis::y]);
	mpu.setZGyroOffset(_gyroOffsets[_axis::z]);
}

void BackSeatAngleTracker::CalculateAccelerationsMean(MPU6050 &mpu)
{
	const int numberOfDiscardedMeasures = 100;
	const unsigned int timeBetweenMeasures = 2000;

	uint16_t i = 0;
	int accBuffer[3] = {0, 0, 0};
	int gBuffer[3] = {0, 0, 0};
	int16_t _ax, _ay, _az;
	int16_t _gx, _gy, _gz;

	while (i < (BUFFER_SIZE + numberOfDiscardedMeasures))
	{
		mpu.getMotion6(&_ax, &_ay, &_az, &_gx, &_gy, &_gz);

		if (i > numberOfDiscardedMeasures)
		{
			accBuffer[_axis::x] = accBuffer[_axis::x] + _ax;
			accBuffer[_axis::y] = accBuffer[_axis::y] + _ay;
			accBuffer[_axis::z] = accBuffer[_axis::z] + _az;

			gBuffer[_axis::x] = gBuffer[_axis::x] + _gx;
			gBuffer[_axis::y] = gBuffer[_axis::y] + _gy;
			gBuffer[_axis::z] = gBuffer[_axis::z] + _gz;
		}

		i++;
		usleep(timeBetweenMeasures);
	}

	for (uint8_t j = 0; j < 3; j++)
	{
		_accelerationMeans[j] = accBuffer[j] / BUFFER_SIZE;
		_gyroMeans[j] = gBuffer[j] / BUFFER_SIZE;
	}
}

void BackSeatAngleTracker::Calibrate(MPU6050 &mpu)
{
	for (uint8_t i = 0; i < 3; i++)
	{
		_accelerationOffsets[i] = (_calibrationArray[i] - _accelerationMeans[i]) / 8;
		_gyroOffsets[i] = -_gyroMeans[i] / 4;
	}

	uint8_t ready = 0;

	while (ready < 6)
	{
		ready = 0;

		SetImuOffsets(mpu);
		CalculateAccelerationsMean(mpu);

		for (uint8_t i = 0; i < 3; i++)
		{
			printf("%i\t", abs(_calibrationArray[i] - _accelerationMeans[i]));
			printf("%i\n", abs(_gyroMeans[i]));

			if (abs(_calibrationArray[i] - _accelerationMeans[i]) <= ACCELEROMETER_DEADZONE)
			{
				ready++;
			}
			else
			{
				_accelerationOffsets[i] = _accelerationOffsets[i] + (_calibrationArray[i] - _accelerationMeans[i]) / ACCELEROMETER_DEADZONE;
			}

			if (abs(_gyroMeans[i]) <= GYROSCOPE_DEADZONE)
			{
				ready++;
			}
			else
			{
				_gyroOffsets[i] = _gyroOffsets[i] - _gyroMeans[i] / (GYROSCOPE_DEADZONE + 1);
			}
		}
	}
}

void BackSeatAngleTracker::GetMPUAccelations(MPU6050 &mpu, double *realAccelerations)
{
	int16_t _ax, _ay, _az;
	mpu.getAcceleration(&_ax, &_ay, &_az);

	// TODO: Add low-pass filter

	// Décomenter pour debug
	// printf("ax: %i\n", _ax);
	// printf("ay: %i\n", _ay);
	// printf("az: %i\n", _az);

	realAccelerations[_axis::x] = double(_ax) * 2 / 32768.0f;
	realAccelerations[_axis::y] = double(_ay) * 2 / 32768.0f;
	realAccelerations[_axis::z] = double(_az) * 2 / 32768.0f;
}

double BackSeatAngleTracker::GetPitch(double accelerations[])
{
	// Décomenter pour debug
	// printf("datatable ax: %f\n", accelerations[0]);
	// printf("datatable ay: %f\n", accelerations[1]);
	// printf("datatable az: %f\n", accelerations[2]);

	return (atan2(accelerations[0], sqrt(accelerations[1] * accelerations[1] + accelerations[2] * accelerations[2])) * 180.0) / M_PI;
}

double BackSeatAngleTracker::GetRoll(double accelerations[])
{
	return -1 * (atan2(-accelerations[1], accelerations[2]) * 180.0) / M_PI;
}

int BackSeatAngleTracker::GetBackSeatAngle()
{
	double fixedImuAccelerations[3] = {0, 0, 0};
	double mobileImuAccelerations[3] = {0, 0, 0};

	GetMPUAccelations(_fixedImu, fixedImuAccelerations);
	GetMPUAccelations(_mobileImu, mobileImuAccelerations);

	double fixedPitch = GetPitch(fixedImuAccelerations);
	double mobilePitch = GetPitch(mobileImuAccelerations);
	// Valeur absolue est temporaire en attendant que l'acceleromètre soit fixé.
	// TODO remove abs
	int angle = abs(int(fixedPitch - mobilePitch));

	// printf("The mobile pitch: %f\n", mobilePitch);
	// printf("The fixed pitch is: %f\n", fixedPitch);
	// printf("The backseat angle is: %i\n", angle);
	return angle;
}