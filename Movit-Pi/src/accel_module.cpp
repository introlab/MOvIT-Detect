#include "accel_module.h"
#include <unistd.h>
#include <math.h>

BackSeatAngleTracker::BackSeatAngleTracker()
{
	Initialize();
}

void BackSeatAngleTracker::Initialize()
{
	InitializeFixedImu();
	InitializeMobileImu();
}

void BackSeatAngleTracker::InitializeFixedImu()
{
	if (!_fixedImu.testConnection())
	{
		printf("Failed to initialize the fixed imu...\n");
		return;
	}

	_fixedImu.initialize();

	SetCalibrationArray(_axis::x);
	ResetIMUOffsets(_fixedImu);
	CalculateAccelerationsMean(_fixedImu);
	Calibrate(_fixedImu);
	CalculateAccelerationsMean(_fixedImu);

	printf("FINISHED!\n");
	printf("Your offsets:\t");
	printf("%i", _accelerationOffsets[0]);
	printf("\t");
	printf("%i", _accelerationOffsets[1]);
	printf("\t");
	printf("%i", _accelerationOffsets[2]);
	printf("\t");
	printf("%i", _gyroOffsets[0]);
	printf("\t");
	printf("%i", _gyroOffsets[1]);
	printf("\t");
	printf("%i\n", _gyroOffsets[2]);

	SetImuOffsets(_fixedImu);
}

void BackSeatAngleTracker::InitializeMobileImu()
{
	if (!_mobileImu.testConnection())
	{
		printf("Failed to initialize the mobile imu...\n");
		return;
	}

	_mobileImu.initialize();

	SetCalibrationArray(_axis::x);
	ResetIMUOffsets(_mobileImu);
	CalculateAccelerationsMean(_mobileImu);

	// Calculating offsets: 
	Calibrate(_mobileImu);
	CalculateAccelerationsMean(_mobileImu);

	printf("FINISHED!\n");
	printf("Your offsets:\t");
	printf("%i", _accelerationOffsets[0]);
	printf("\t");
	printf("%i", _accelerationOffsets[1]);
	printf("\t");
	printf("%i", _accelerationOffsets[2]);
	printf("\t");
	printf("%i", _gyroOffsets[0]);
	printf("\t");
	printf("%i", _gyroOffsets[1]);
	printf("\t");
	printf("%i\n", _gyroOffsets[2]);

	SetImuOffsets(_mobileImu);
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
			printf("%i\n", abs(_calibrationArray[i] - _accelerationMeans[i]));
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

	printf("ax: %i\n", _ax);
	printf("ay: %i\n", _ay);
	printf("az: %i\n", _az);

	realAccelerations[_axis::x] = double(_ax) * 2 / 32768.0f;
	realAccelerations[_axis::y] = double(_ay) * 2 / 32768.0f;
	realAccelerations[_axis::z] = double(_az) * 2 / 32768.0f;
}

double BackSeatAngleTracker::GetPitch(double accelerations[])
{
	printf("datatable ax: %f\n", accelerations[0]);
	printf("datatable ay: %f\n", accelerations[1]);
	printf("datatable az: %f\n", accelerations[2]);

	return (atan2(accelerations[0], sqrt(accelerations[1] * accelerations[1] + accelerations[2] * accelerations[2])) * 180.0) / M_PI;
}

double BackSeatAngleTracker::GetRoll(double accelerations[])
{
	return -1 * (atan2(-accelerations[1], accelerations[2]) * 180.0) / M_PI;
}

int BackSeatAngleTracker::GetBackSeatAngle()
{
	double fixedImuAccelerations[3] = {0, 0, 0};
	double mobileImuAccelerations[3] = { 0, 0, 0 };

	GetMPUAccelations(_fixedImu, fixedImuAccelerations);
	GetMPUAccelations(_mobileImu, mobileImuAccelerations);

	double fixedPitch = GetPitch(fixedImuAccelerations);
	double mobilePitch = GetPitch(mobileImuAccelerations);
	int angle = int(fixedPitch - mobilePitch);

	printf("The mobile pitch: %f\n", mobilePitch);
	printf("The fixed pitch is: %f\n", fixedPitch);
	printf("The backseat angle is: %i\n", angle);
	return angle;
}