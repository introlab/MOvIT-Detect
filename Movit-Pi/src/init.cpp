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
#include "PCA9536.h"  //Arduino 4 programmable 4-channel digital i-o

//Include : Modules
#include "init.h"         //variables and modules initialisation
#include "notif_module.h" //variables and modules initialisation
#include "accel_module.h" //variables and modules initialisation
#include "force_module.h" //variables and modules initialisation
#include "program.h"      //variables and modules initialisation
#include "test.h"         //variables and modules initialisation

//External functions and variables
//Variables
extern MPU6050 imuMobile;             //Initialisation of the mobile MPU6050
extern MPU6050 imuFixe;               //Initialisation of the fixed MPU6050
extern MAX11611 max11611;             //Initialisation of the 10-bit ADC
extern PCA9536 pca9536;               //Construct a new PCA9536 instance
extern uint16_t *max11611Data;        //ADC 10-bit data variable
extern int capteurForceNb;            //Total number of sensors
extern uint16_t max11611DataArray[9]; //Data table of size=total sensors
//Functions
extern void calibrationProcess(MPU6050 &mpu, uint8_t calibrationComplexite);

void init_accel()
{
    //Accelerometer setup

    printf("imuMobile.getDeviceID() = %X\n", imuMobile.getDeviceID());

    if (!imuFixe.testConnection())
    {
        printf("imuFixe not found.\n");
    }
    else
    {
        printf("imuFixe.initialize()\n");
        imuFixe.initialize();
        printf(".-.--..---.-.-.--.--.--.---.--.-");
        printf("\nimuFixe initialized...\n");
        //calibrationProcess(imuFixe, 1); // Décommenter pour une calibration à chaque début de programme.
        printf("imuFixe calibrated.\n");
        printf(".-.--..---.-.-.--.--.--.---.--.-");
        printf("\n");
        // Dernier offset mesurés le 12 janvier 2018 - 17h10
        // ax   ay    az    gx  gy  gz
        // 806  -4419 1018  264 -74 15
    }
    if (!imuMobile.testConnection())
    {
        printf("imuMobile not found.\n");
    }
    else
    {
        imuMobile.initialize();
        printf(".-.--..---.-.-.--.--.--.---.--.-");
        printf("\nimuMobile initialized...\n");
        //calibrationProcess(imuMobile, 1); // Décommenter pour une calibration à chaque début de programme.
        printf("imuMobile calibrated.\n");
        printf(".-.--..---.-.-.--.--.--.---.--.-");
        printf("\n");
        // Dernier offset mesurés le 12 janvier 2018 - 17h10
        // ax   ay    az    gx  gy  gz
        // -700 -1054 1562  76  65  0
    }
}

void init_ADC()
{
    //TODO: Ajouter une facon de détecter si l'initialisation a fail
    max11611.initialize();
    printf("max11611 initialized...\n");

    for (unsigned int i = 0; i < sizeof(max11611DataArray); i++)
    {
        max11611DataArray[i] = 0;
    }
    max11611Data = max11611DataArray; //pointer assignation to the data table
}

void init_PCA9536()
{
    delay(50);
    pca9536.reset();
    pca9536.setMode(DC_MOTOR, IO_OUTPUT);
    pca9536.setMode(GREEN_LED, IO_OUTPUT);
    pca9536.setMode(RED_LED, IO_OUTPUT);
    pca9536.setState(IO_LOW);
    pca9536.setMode(PUSH_BUTTON, IO_INPUT);
    pca9536.setPolarity(PUSH_BUTTON, IO_INVERTED);

    if (pca9536.getMode(DC_MOTOR) == IO_OUTPUT && pca9536.getMode(GREEN_LED) == IO_OUTPUT && pca9536.getMode(RED_LED) == IO_OUTPUT)
    {
        printf("PCA9536 initialized success...\n");
    }
    else
    {
        printf("PCA9536 initialized failed...\n");
    }

}

void init_notification()
{
    StopBuzzer();
    LightOff(RED_LED);
    LightOff(GREEN_LED);
}
