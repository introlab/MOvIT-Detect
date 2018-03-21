//---------------------------------------------------------------------------------------
// TITRE
// Description
//---------------------------------------------------------------------------------------

#include "MPU6050.h"  //Implementation of Jeff Rowberg's driver
#include "MAX11611.h" //10-Bit ADC librairy

#include "init.h"         //variables and modules initialisation
#include "alarm.h" //variables and modules initialisation
#include "accel_module.h" //variables and modules initialisation
#include "forceSensor.h"   //variables and modules initialisation
#include "forcePlate.h"   //variables and modules initialisation
#include "program.h"      //variables and modules initialisation
#include "test.h"         //variables and modules initialisation

extern MPU6050 imuMobile;             //Initialisation of the mobile MPU6050
extern MPU6050 imuFixe;               //Initialisation of the fixed MPU6050
extern MAX11611 max11611;             //Initialisation of the 10-bit ADC

void init_ADC(forceSensor &sensorMatrix)
{
    printf("MAX11611 (ADC) initializing ... ");
    if (max11611.initialize())
    {
        for (uint8_t i = 0; i < sensorMatrix.sensorCount; i++)
        {
            sensorMatrix.SetAnalogData(0, i);
        }
        // La ligne suivante est commenté car elle cause une division par 0
        sensorMatrix.GetForceSensorData(sensorMatrix);
        printf("success\n");
    }
    else
    {
        printf("FAIL\n");
    }
}
