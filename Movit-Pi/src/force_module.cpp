//---------------------------------------------------------------------------------------
// TITRE
// Description
//---------------------------------------------------------------------------------------

//Include : Drivers
#include "MPU6050.h"  //Implementation of Jeff Rowberg's driver
#include "MAX11611.h" //10-Bit ADC

//Include : Modules
#include "init.h"         //variables and modules initialisation
#include "notif_module.h" //variables and modules initialisation
#include "accel_module.h" //variables and modules initialisation
#include "force_module.h" //variables and modules initialisation
#include "program.h"      //variables and modules initialisation
#include "test.h"         //variables and modules initialisation

//ADC variables
uint16_t *max11611Data;                     //ADC 10-bit data variable
const uint8_t capteurForceNb = 9;          //Total number of sensors
uint16_t max11611DataArray[capteurForceNb]; //Data table of size=total sensors
long max11611Voltage[capteurForceNb];       //ADC 10-bit data variable
long max11611Resistance[capteurForceNb];    //ADC 10-bit data variable
long max11611Conductance[capteurForceNb];   //ADC 10-bit data variable
long max11611Force[capteurForceNb];         //ADC 10-bit data variable


bool left_shearing = false;
bool right_shearing = false;
bool front_shearing = false;
bool rear_shearing = false;


long map(long x, long in_min, long in_max, long out_min, long out_max);

//---------------------------------------------------------------------------------------
// FUNCTION POUR VERIFIER SI QUELQUUN EST ASSIS SUR LA CHAISE
// Bool state function, return 1-0
//---------------------------------------------------------------------------------------
bool isSomeoneThere()
{
    /***************************** FRS MAP *****************************/
    /* FRONT LEFT                                          FRONT RIGHT */
    /* max11611Data[2]         max11611Data[1]         max11611Data[9] */
    /* max11611Data[5]         max11611Data[4]         max11611Data[3] */
    /* max11611Data[8]         max11611Data[7]         max11611Data[6] */
    /*******************************************************************/

    // Ajouter dans le fichier d'init des variables, comme avec capteurForceNb ? - LP
    const double threshold = 1024 * 9 / 5;
    const double sideThreshold = 1024 * 3 / 5;
    double value = 0.0;
    double leftSideValue = 0.0;
    double rightSideValue = 0.0;

    // Obtention de la somme des valeurs de tous les capteurs
    for (uint8_t capteurNo = 0; capteurNo < capteurForceNb; capteurNo++)
    {
        value += max11611Data[capteurNo];
    }

    // Threshold is one fifth of one channel's maximal value :: Est-ce que le threshold est setter de façon arbitraire ? - LP
    leftSideValue = max11611Data[2] + max11611Data[5] + max11611Data[8];
    rightSideValue = max11611Data[9] + max11611Data[3] + max11611Data[6];

    // Triple verification pour detecter une présence sur le fauteuil
    if (leftSideValue >= sideThreshold && rightSideValue >= sideThreshold && value >= threshold)
    {
        return true;
    }
    else
    {
        return false;
    }
}

long map(long x, long in_min, long in_max, long out_min, long out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void ForceSensorUnits()
{
    // https://learn.adafruit.com/force-sensitive-resistor-fsr/using-an-fsr
    // The voltage = Vcc * R / (R + FSR) where R = 10K and Vcc = 5V
    // so FSR = ((Vcc - V) * R) / V
    for (uint8_t capteurNo = 0; capteurNo < capteurForceNb; capteurNo++)
    {
        max11611Voltage[capteurNo] = map(max11611Data[capteurNo], 0, 1023, 0, 5000);

        max11611Resistance[capteurNo] = 5000 - max11611Voltage[capteurNo]; // fsrVoltage is in millivolts so 5V = 5000mV
        max11611Resistance[capteurNo] *= 10000;                            // 10K resistor
        max11611Resistance[capteurNo] /= max11611Voltage[capteurNo];

        max11611Conductance[capteurNo] = 1000000; // we measure in micromhos so
        max11611Conductance[capteurNo] /= max11611Resistance[capteurNo];

        if (max11611Conductance[capteurNo] <= 1000)
        {
            max11611Force[capteurNo] = max11611Conductance[capteurNo] / 80;
        }
        else
        {
            max11611Force[capteurNo] = max11611Conductance[capteurNo] - 1000;
            max11611Force[capteurNo] /= 30;
        }
    }
}

void shearing_detection1()
{
    //Shearing detection variables
    double left_sensors = 0.0;
    double right_sensors = 0.0;
    double front_sensors = 0.0;
    double rear_sensors = 0.0;
    double center_LR_sensors = 0.0; 
    double center_FR_sensors = 0.0;

    //Left-Right values
    center_LR_sensors = max11611Data[1] + max11611Data[4] + max11611Data[7];
    left_sensors = max11611Data[2] + max11611Data[5] + max11611Data[8];
    right_sensors = max11611Data[9] + max11611Data[3] + max11611Data[6];

    //Front-Rear values
    center_FR_sensors = max11611Data[5] + max11611Data[4] + max11611Data[3];
    front_sensors = max11611Data[2] + max11611Data[1] + max11611Data[9];
    rear_sensors = max11611Data[8] + max11611Data[7] + max11611Data[6];

    //Left-Right (LR) shearing detection
    if (isSomeoneThere())
    {
        if (left_sensors > (right_sensors + 0.05 * right_sensors))
        {
            left_shearing = true;
        }
        else if (right_sensors > (left_sensors + 0.05 * left_sensors))
        {
            right_shearing = true;
        }

        //Front-Rear (FR) shearing detection
        if (front_sensors > (rear_sensors + 0.05 * rear_sensors))
        {
            front_shearing = true;
        }
        else if (rear_sensors > (front_sensors + 0.05 * front_sensors))
        {
            rear_shearing = true;
        }
    }
}

void ForcePlate::CreateForcePlate(ForcePlate &NewForcePlate, int SensorNo1, int SensorNo2, int SensorNo3, int SensorNo4)
//---------------------------------------------------------------------------------------
//Function: CreateForcePlate
//Force plate creation from 2x2 force sensors matrix
//Centor of Pressure calculation and Coefficient of Friction
//Reference: Kistler force plate formulae PDF
//---------------------------------------------------------------------------------------
{
    /********FORCE PLATE MAP ********/
    /* FRONT LEFT       FRONT RIGHT */
    /* SensorNo3         SensorNo4  */
    /* SensorNo2         SensorNo1  */
    /********************************/

    //Force plate output signals
    NewForcePlate.fx12 = max11611Data[SensorNo1] + max11611Data[SensorNo2];
    NewForcePlate.fx34 = max11611Data[SensorNo3] + max11611Data[SensorNo4];
    NewForcePlate.fy14 = max11611Data[SensorNo1] + max11611Data[SensorNo4];
    NewForcePlate.fy23 = max11611Data[SensorNo2] + max11611Data[SensorNo3];
    NewForcePlate.fz1 = max11611Data[SensorNo1];
    NewForcePlate.fz2 = max11611Data[SensorNo2];
    NewForcePlate.fz3 = max11611Data[SensorNo3];
    NewForcePlate.fz4 = max11611Data[SensorNo4];

    //Force sensors positioning
    NewForcePlate.a = 4 / 2;   //Distance along X axis from SensorNo1 to SensorNo2
    NewForcePlate.b = 7.5 / 2; //Distance along Y axis from SensorNo2 to SensorNo3
    NewForcePlate.az0 = 0.5;   //Half of the force plate height : 0.5cm approximate for plexiglass? VALIDATE

    //Calculated parameters
    NewForcePlate.Fx = fx12 + fx34;
    NewForcePlate.Fy = fy14 + fy23;
    NewForcePlate.Fz = fz1 + fz2 + fz3 + fz4;
    NewForcePlate.Mx = b * (fz1 + fz2 - fz3 - fz4);
    NewForcePlate.My = a * (-fz1 + fz2 + fz3 - fz4);
    NewForcePlate.Mz = b * (-fx12 + fx34) + a * (fy14 - fy23);
    NewForcePlate.Mx1 = Mx + Fy * az0;
    NewForcePlate.My1 = My - Fx * az0;

    //Coordinate of the force application point (C.O.P.)
    NewForcePlate.COPx = -My1 / Fz;
    NewForcePlate.COPy = Mx1 / Fz;

    //Coefficients of friction
    NewForcePlate.COFx = Fx / Fz;
    NewForcePlate.COFy = Fy / Fz;
    NewForcePlate.COFxy = sqrt(COFx * COFx + COFy * COFy);
}

void ForcePlate::AnalyzeForcePlates(ForcePlate &GlobalForcePlate, ForcePlate &ForcePlate1, ForcePlate &ForcePlate2, ForcePlate &ForcePlate3, ForcePlate &ForcePlate4)
//---------------------------------------------------------------------------------------
//Function: AnalyzeForcePlates
//Global coordinate system (treat multiple force plates as one)
//CentorOfPressure calculation
//Reference: Kistler force plate formulae PDF
//---------------------------------------------------------------------------------------
{
    /*********FORCE PLATES MAP ******************  y  */
    /* FRONT LEFT           FRONT RIGHT            |  */
    /* ForcePlate3         ForcePlate4             |  */
    /* ForcePlate2         ForcePlate1    x<-------|  */
    /**************************************************/

    //Force plates positioning
    int dax = ForcePlate1.a; //X-Axis distance between force sensors
    int day = ForcePlate1.b; //Y-Axis distance between force sensors
    int az0 = ForcePlate1.az0;

    int dax1 = -dax; //Force plate 1 : bottom-right quadrant
    int day1 = -day; //(-x, -y)
    int dax2 = 0;    //Force plate 2 : bottom-left quadrant
    int day2 = -day; //(0, -y)
    int dax3 = 0;    //Force plate 3 : top-left quadrant
    int day3 = 0;    //(0, 0)
    int dax4 = -dax; //Force plate 4 : top-right quadrant
    int day4 = 0;    //(-x, 0)

    //Calculated parameters
    GlobalForcePlate.Fx = ForcePlate1.Fx + ForcePlate2.Fx + ForcePlate3.Fx + ForcePlate4.Fx;
    GlobalForcePlate.Fy = ForcePlate1.Fy + ForcePlate2.Fy + ForcePlate3.Fy + ForcePlate4.Fy;
    GlobalForcePlate.Fz = ForcePlate1.Fz + ForcePlate2.Fz + ForcePlate3.Fz + ForcePlate4.Fz;
    GlobalForcePlate.Mx = (day1 + day) * ForcePlate1.Fz + (day2 + day) * ForcePlate2.Fz + (day3 + day) * ForcePlate3.Fz + (day4 + day) * ForcePlate4.Fz;
    GlobalForcePlate.My = -(dax1 + dax) * ForcePlate1.Fz - (dax2 + dax) * ForcePlate2.Fz - (dax3 + dax) * ForcePlate3.Fz - (dax4 + dax) * ForcePlate4.Fz;
    GlobalForcePlate.Mx1 = Mx + az0 * ForcePlate1.Fy + az0 * ForcePlate2.Fy + az0 * ForcePlate3.Fy + az0 * ForcePlate4.Fy;
    GlobalForcePlate.My1 = My - az0 * ForcePlate1.Fx - az0 * ForcePlate2.Fx - az0 * ForcePlate3.Fx - az0 * ForcePlate4.Fx;

    //Coordinate of the force application point (C.O.P.)
    GlobalForcePlate.COPx = -GlobalForcePlate.My1 / GlobalForcePlate.Fz; //X-Coordinate of the global application point
    GlobalForcePlate.COPy = GlobalForcePlate.Mx1 / GlobalForcePlate.Fz;  //Y-Coordinate of the global application point
}
