//---------------------------------------------------------------------------------------
// TITRE
// Description
//---------------------------------------------------------------------------------------

//Include : Drivers
#include "MPU6050.h" //Implementation of Jeff Rowberg's driver

//Include : Modules
#include "init.h"         //variables and modules initialisation
#include "notif_module.h" //variables and modules initialisation
#include "accel_module.h" //variables and modules initialisation
#include "program.h"      //variables and modules initialisation
#include "test.h"         //variables and modules initialisation

//External functions and variables
//Variables
extern MPU6050 imuMobile; //Initialisation of the mobile MPU6050
extern MPU6050 imuFixe;   //Initialisation of the fixed MPU6050

// extern int aRange;                                          //maximal measurable g-force
// extern int16_t ax, ay, az;                                  //axis accelerations
// extern double aRawFixe[3], aRawMobile[3];                   //raw acceleration
// extern double aRealFixe[3], aRealMobile[3];                 //computed acceleration
// extern double pitchFixe, rollFixe, pitchMobile, rollMobile; //pitch and roll angle values
// extern int angle;                                           //Final angle computed between sensors
// extern int obs;                                             //Debug variable for getAngle function
// extern int mean_ax, mean_ay, mean_az, mean_gx, mean_gy, mean_gz, state;
// extern int ax_offset, ay_offset, az_offset, gx_offset, gy_offset, gz_offset;
// extern int16_t gx, gy, gz; //axis angles

//Accelerometer variables
int aRange = 2;                                              //maximal measurable g-force
int16_t ax, ay, az;                                          //axis accelerations
double aRawFixe[3] = {0, 0, 0}, aRawMobile[3] = {0, 0, 0};   //raw acceleration
double aRealFixe[3] = {0, 0, 0}, aRealMobile[3] = {0, 0, 0}; //computed acceleration
double pitchFixe, rollFixe, pitchMobile, rollMobile;         //pitch and roll angle values
int angle = 0;                                               //Final angle computed between sensors
int obs = 3;                                                 //Debug variable for getAngle function

//Variables d'ajustement des IMU
//Definir localement
int mean_ax, mean_ay, mean_az, mean_gx, mean_gy, mean_gz, state = 0;
int ax_offset, ay_offset, az_offset, gx_offset, gy_offset, gz_offset;
int flagRunning = 0;

//Gyroscope variables
int16_t gx, gy, gz; //axis angles

// extern SimpleTimer timer;         //Creation of a timer
extern unsigned char dateTime[7]; //{s, m, h, w, d, date, month, year}

extern double tempArray[3], dump[3], tempAcc[3];
const float isMovingTrigger = 1.05;
extern float isMovingValue;
//Function
extern void getMPUAccData(MPU6050 &mpu, double *aRaw, double *aReal);

///////////////////////////////////   CONFIGURATION   /////////////////////////////
//Change this 3 variables if you want to fine tune the skecth to your needs.
int buffersize = 1000; //Amount of readings used to average, make it higher to get more precision but sketch will be slower  (default:1000)
int acel_deadzone = 8; //Acelerometer error allowed, make it lower to get more precision, but sketch may not converge  (default:8)
int giro_deadzone = 1; //Giro error allowed, make it lower to get more precision, but sketch may not converge  (default:1)

int calib_array[3] = {0, 0, 0};

///////////////////////////////////   FUNCTIONS   ////////////////////////////////////

void calibrationProcess(MPU6050 &mpu, uint8_t axis_config)
{

    if (mpu.testConnection())
    {
        printf("MPU6050 connection successful\n");
    }
    else
    {
        printf("MPU6050 connection failed\n");
    }

    if (axis_config == 1)
    { // if X axis is pointing down
        calib_array[0] = -16384;
        calib_array[1] = 0;
        calib_array[2] = 0;
    }
    else if (axis_config == 2)
    { // if Y axis is pointing down
        calib_array[0] = 0;
        calib_array[1] = -16384;
        calib_array[2] = 0;
    }
    else if (axis_config == 3)
    { // if Z axis is pointing down
        calib_array[0] = 0;
        calib_array[1] = 0;
        calib_array[2] = -16384;
    }

    resetIMUOffsets(mpu);
    calibrationIMU(mpu);
    printf("Wait end\n");
}

void resetIMUOffsets(MPU6050 &mpu)
{
    // reset offsets
    mpu.setXAccelOffset(0);
    mpu.setYAccelOffset(0);
    mpu.setZAccelOffset(0);
    mpu.setXGyroOffset(0);
    mpu.setYGyroOffset(0);
    mpu.setZGyroOffset(0);
}

void calibrationIMU(MPU6050 &mpu)
{

    if (state == 0)
    {
        printf("Reading sensors for first time...\n");
        meansensors(mpu);
        state++;
    }

    if (state == 1)
    {
        printf("Calculating offsets...\n");
        calibration(mpu);
        state++;
    }

    if (state == 2)
    {
        meansensors(mpu);
        printf("FINISHED!\n");
        printf("Sensor readings with offsets:\t");
        printf("%i", mean_ax);
        printf("\t");
        printf("%i", mean_ay);
        printf("\t");
        printf("%i", mean_az);
        printf("\t");
        printf("%i", mean_gx);
        printf("\t");
        printf("%i", mean_gy);
        printf("\t");
        printf("%i\n", mean_gz);
        printf("Your offsets:\t");
        printf("%i", ax_offset);
        printf("\t");
        printf("%i", ay_offset);
        printf("\t");
        printf("%i", az_offset);
        printf("\t");
        printf("%i", gx_offset);
        printf("\t");
        printf("%i", gy_offset);
        printf("\t");
        printf("%i\n", gz_offset);

        state++;
    }

    if (state == 3)
    {
        mpu.setXAccelOffset(ax_offset);
        mpu.setYAccelOffset(ay_offset);
        mpu.setZAccelOffset(az_offset);

        mpu.setXGyroOffset(gx_offset);
        mpu.setXGyroOffset(gy_offset);
        mpu.setXGyroOffset(gz_offset);

        state = 0;
    }
}

void meansensors(MPU6050 &mpu)
{
    long i = 0, buff_ax = 0, buff_ay = 0, buff_az = 0, buff_gx = 0, buff_gy = 0, buff_gz = 0;

    while (i < (buffersize + 101))
    {
        // read raw accel/gyro measurements from device
        mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

        if (i > 100 && i <= (buffersize + 100))
        { //First 100 measures are discarded
            buff_ax = buff_ax + ax;
            buff_ay = buff_ay + ay;
            buff_az = buff_az + az;
            buff_gx = buff_gx + gx;
            buff_gy = buff_gy + gy;
            buff_gz = buff_gz + gz;
        }
        if (i == (buffersize + 100))
        {
            mean_ax = buff_ax / buffersize;
            mean_ay = buff_ay / buffersize;
            mean_az = buff_az / buffersize;
            mean_gx = buff_gx / buffersize;
            mean_gy = buff_gy / buffersize;
            mean_gz = buff_gz / buffersize;
        }
        i++;
        delay(2); //Needed so we don't get repeated measures
    }
}

void calibration(MPU6050 &mpu)
{
    ax_offset = (calib_array[0] - mean_ax) / 8;
    ay_offset = (calib_array[1] - mean_ay) / 8;
    az_offset = (calib_array[2] - mean_az) / 8;

    gx_offset = -mean_gx / 4;
    gy_offset = -mean_gy / 4;
    gz_offset = -mean_gz / 4;
    while (1)
    {
        int ready = 0;
        mpu.setXAccelOffset(ax_offset);
        mpu.setYAccelOffset(ay_offset);
        mpu.setZAccelOffset(az_offset);

        mpu.setXGyroOffset(gx_offset);
        mpu.setYGyroOffset(gy_offset);
        mpu.setZGyroOffset(gz_offset);

        meansensors(mpu);
        printf("...\n");

        if (abs(calib_array[0] - mean_ax) <= acel_deadzone)
            ready++;
        else
            ax_offset = ax_offset + (calib_array[0] - mean_ax) / acel_deadzone;

        if (abs(calib_array[1] - mean_ay) <= acel_deadzone)
            ready++;
        else
            ay_offset = ay_offset + (calib_array[1] - mean_ay) / acel_deadzone;

        if (abs(calib_array[2] - mean_az) <= acel_deadzone)
            ready++;
        else
            az_offset = az_offset + (calib_array[2] - mean_az) / acel_deadzone;

        if (abs(mean_gx) <= giro_deadzone)
            ready++;
        else
            gx_offset = gx_offset - mean_gx / (giro_deadzone + 1);

        if (abs(mean_gy) <= giro_deadzone)
            ready++;
        else
            gy_offset = gy_offset - mean_gy / (giro_deadzone + 1);

        if (abs(mean_gz) <= giro_deadzone)
            ready++;
        else
            gz_offset = gz_offset - mean_gz / (giro_deadzone + 1);

        if (ready == 6)
            break;
    }
}

//Accelerometer fonctions
//Get the raw data and transform them in real data
void getMPUAccData(MPU6050 &mpu, double *aRaw, double *aReal)
{
    mpu.getAcceleration(&ax, &ay, &az);

    //Need to add the low-pass filter here

    aRaw[0] = ax;
    aRaw[1] = ay;
    aRaw[2] = az;

    aReal[0] = double(ax) * aRange / 32768;
    aReal[1] = double(ay) * aRange / 32768;
    aReal[2] = double(az) * aRange / 32768;
}

//Get pitch and roll using the real data (the getMPUAccData fonction need to be called before using this fonction)
void getMPUypr(double *pitch, double *roll, double dataTable[])
{
    *pitch = (atan2(dataTable[0], sqrt(dataTable[1] * dataTable[1] + dataTable[2] * dataTable[2])) * 180.0) / M_PI;
    *roll = -1 * (atan2(-dataTable[1], dataTable[2]) * 180.0) / M_PI;
}

void getAngle()
{
    //Suivant l'installation des modules, la comparaison doit se faire entre le pitch et/ou le roll des 2 capteurs
    switch (obs)
    {
    case 1: //Pitch avec Pitch
        angle = int(pitchFixe) - int(pitchMobile);
        break;
    case 2: //Pitch avec Roll
        angle = int(rollFixe - pitchMobile);
        break;
    case 3: //Roll avec Pitch
        angle = int(pitchFixe - pitchMobile);
        break;
    case 4: //Roll avec Roll
        angle = int(rollFixe - rollMobile);
        break;
    default:
        break;
    }
}

//---------------------------------------------------------------------------------------
// FUNCTION POUR VERIFIER SI LE FAUTEUIL EST EN MOUVEMENT
// Bool state function, return 1-0
//---------------------------------------------------------------------------------------
bool isMoving()
{
    // Ajouter dans le fichier d'init des variables? - LP
    double AccAct;
    double AccPrev;

    getMPUAccData(imuFixe, dump, tempArray);
    tempAcc[0] = tempAcc[1];
    tempAcc[1] = tempAcc[2];

    double value = (tempArray[0] * tempArray[0] + tempArray[1] * tempArray[1] + tempArray[2] * tempArray[2]) * 9.80665;
    tempAcc[2] = sqrt(value); // m * s^-2

    //A verifier...

    if (tempAcc[0] != 0)
    {
        AccPrev = tempAcc[1] - tempAcc[0];
        AccAct = tempAcc[2] - tempAcc[1];
    }
    else
    {
        AccAct = 0.0;
        AccPrev = 0.0;
    }
    isMovingValue = AccAct - AccPrev;

    if (abs(isMovingValue) > isMovingTrigger)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
