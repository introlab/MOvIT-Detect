#ifndef _ACCEL_MODULE_H_
#define _ACCEL_MODULE_H_

#include "MPU6050.h"  //Implementation of Jeff Rowberg's driver

//fichier b_IMUcalibration_v5.ino
void calibrationProcess(MPU6050 &mpu, uint8_t calibrationComplexite);
void resetIMUOffsets(MPU6050 &mpu);
void calibrationIMU(MPU6050 &mpu);
void meansensors(MPU6050 &mpu);
void calibration(MPU6050 &mpu);

//function user
void getMPUAccData(MPU6050 &mpu, double *aRaw, double *aReal);
void getMPUypr(double *pitch, double *roll, double  dataTable[]);
void getAngle();
bool isMoving();

#endif /* _ACCEL_MODULE_H_ */
