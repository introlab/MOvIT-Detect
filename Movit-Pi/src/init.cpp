//---------------------------------------------------------------------------------------
// TITRE
// Description
//---------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------
// TITLE
// Includes : Drivers Modules
//---------------------------------------------------------------------------------------
//Include : Drivers
#include "MPU6050.h"  //Implementation of Jeff Rowberg's driver
#include "MAX11611.h" //10-Bit ADC librairy

//Include : Modules
#include "init.h"         //variables and modules initialisation
#include "notif_module.h" //variables and modules initialisation
#include "accel_module.h" //variables and modules initialisation
#include "ForceSensor.h"  //variables and modules initialisation
#include "program.h"      //variables and modules initialisation
#include "test.h"         //variables and modules initialisation


#include <stdio.h>

//External functions and variables
//Variables
extern MPU6050 imuMobile;             //Initialisation of the mobile MPU6050
extern MPU6050 imuFixe;               //Initialisation of the fixed MPU6050
extern MAX11611 max11611;             //Initialisation of the 10-bit ADC
extern uint16_t *max11611Data;        //ADC 10-bit data variable
extern uint16_t max11611DataArray[sensorCount]; //Data table of size=total sensors
//Functions
extern void calibrationProcess(MPU6050 &mpu, uint8_t calibrationComplexite);

extern ForceSensor sensorMatrix;

void init_accel()
{
    //Accelerometer setup

    // printf("imuMobile.getDeviceID() = %X\n", imuMobile.getDeviceID());
    printf("MPU6050 (imuFixe) initializing ... ");
    if (!imuFixe.testConnection())
    {
        printf("FAIL\n");
    }
    else
    {
        imuFixe.initialize();
        printf("success\n");

        //calibrationProcess(imuFixe, 1); // Décommenter pour une calibration à chaque début de programme.

        // printf("imuFixe calibrated.\n");
        // Dernier offset mesurés le 12 janvier 2018 - 17h10
        // ax   ay    az    gx  gy  gz
        // 806  -4419 1018  264 -74 15
    }
    printf("MPU6050 (imuMobile) initializing ... ");
    if (!imuMobile.testConnection())
    {
        printf("FAIL\n");
    }
    else
    {
        imuMobile.initialize();
        printf("success\n");

        //calibrationProcess(imuMobile, 1); // Décommenter pour une calibration à chaque début de programme.

        // printf("imuMobile calibrated.\n");
        // Dernier offset mesurés le 12 janvier 2018 - 17h10
        // ax   ay    az    gx  gy  gz
        // -700 -1054 1562  76  65  0
    }
}

void init_ADC()
{
    printf("MAX11611 (ADC) initializing ... ");
    if (max11611.initialize())
    {
        for (uint8_t i = 0; i < sensorCount; i++)
        {
            sensorMatrix.setAnalogData(0, i);
        }
        // La ligne suivante est commenté car elle cause une division par 0
        // sensorMatrix.GetForceSensorData(sensorMatrix);

        printf("success\n");
    }
    else
    {
        printf("FAIL\n");
    }
}
