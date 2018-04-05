#ifndef _BACK_SEAT_ANGLE_TRACKER_H_
#define _BACK_SEAT_ANGLE_TRACKER_H_

#include "MPU6050.h"

#define ACCELEROMETER_DEADZONE 8 // Accelerometer error allowed, make it lower to get more precision, but sketch may not converge (default: 8)
#define BUFFER_SIZE 1000 // Amount of readings used to average, make it higher to get more precision but sketch will be slower (default: 1000)
#define GYROSCOPE_DEADZONE 1 // Gyroscope error allowed, make it lower to get more precision, but sketch may not converge (default: 1)
#define GRAVITY 9.80665
#define MOVING_TRIGGER 1.05

class BackSeatAngleTracker
{
	private:
		enum _axis { x, y, z };

		MPU6050 _mobileImu = { 0x68 };
		MPU6050 _fixedImu = { 0x69 };

		int _calibrationArray[3] = { 0, 0, 0 };
		int _accelerationMeans[3] = { 0, 0, 0 };
		int _accelerationOffsets[3] = { 0, 0, 0 };
		int _gyroMeans[3] = { 0, 0, 0 };
		int _gyroOffsets[3] = { 0, 0, 0 };

		bool InitializeFixedImu();
		bool InitializeMobileImu();

	public:
		BackSeatAngleTracker();
		bool Initialize();

		void SetCalibrationArray(uint8_t axis);
		void ResetIMUOffsets(MPU6050 &mpu);
		void SetImuOffsets(MPU6050 &mpu);
		void CalculateAccelerationsMean(MPU6050 &mpu);
		void Calibrate(MPU6050 &mpu);

		void GetMPUAccelations(MPU6050 &mpu, double *aReal);
		double GetPitch(double aReal[]);
		double GetRoll(double aReal[]);
		int GetBackSeatAngle();
};

#endif
