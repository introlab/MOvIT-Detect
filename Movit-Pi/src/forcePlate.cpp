//---------------------------------------------------------------------------------------
// HEADER DE FICHIER
// Description
//---------------------------------------------------------------------------------------

#include "MAX11611.h"        //10-Bit ADC
#include "forcePlate.h"
#include "forceSensor.h"

#include <stdio.h>
#include <unistd.h>

extern MAX11611 max11611;                       //Initialisation of the 10-bit ADC
extern uint16_t max11611Data[9];     //Data table of size=total sensors
extern uint16_t max11611EmptyData[9];     //Data table of size=total sensors
extern long sensedPresence;
extern long detectionThreshold;

void forcePlate::CreateForcePlate(forcePlate &newForcePlate, forceSensor &sensors, int sensorNo1, int sensorNo2, int sensorNo3, int sensorNo4)
//---------------------------------------------------------------------------------------
//Function: ForcePlate::CreateForcePlate
//Force plate creation from 2x2 force sensors matrix
//Centor of Pressure calculation and Coefficient of Friction
//Reference: Kistler force plate formulae PDF
//---------------------------------------------------------------------------------------
{
  /********FORCE PLATE MAP ******** y ******** ***/
  /* FRONT LEFT       FRONT RIGHT   |           */
  /* SensorNo3         SensorNo4    |           */
  /* SensorNo2         SensorNo1    |------->x  */
  /**********************************************/

  //Force plate output signals
  newForcePlate.SetFx12(sensors.GetAnalogData(sensorNo1) + sensors.GetAnalogData(sensorNo2));
  newForcePlate.SetFx34(sensors.GetAnalogData(sensorNo3) + sensors.GetAnalogData(sensorNo4));
  newForcePlate.SetFy14(sensors.GetAnalogData(sensorNo1) + sensors.GetAnalogData(sensorNo4));
  newForcePlate.SetFy23(sensors.GetAnalogData(sensorNo2) + sensors.GetAnalogData(sensorNo3));
  newForcePlate.SetFz1(sensors.GetAnalogData(sensorNo1));
  newForcePlate.SetFz2(sensors.GetAnalogData(sensorNo2));
  newForcePlate.SetFz3(sensors.GetAnalogData(sensorNo3));
  newForcePlate.SetFz4(sensors.GetAnalogData(sensorNo4));

  //Calculated parameters
  newForcePlate.SetFx(_fx12 + _fx34);
  newForcePlate.SetFy(_fy14 + _fy23);
  newForcePlate.SetFz(_fz1 + _fz2 + _fz3 + _fz4);
  newForcePlate.SetMx(-distY * (_fz1 + _fz2 - _fz3 - _fz4));
  newForcePlate.SetMy(distX * (-_fz1 + _fz2 + _fz3 - _fz4));
  newForcePlate.SetMz(-distY * (-_fx12 + _fx34) + distX * (_fy14 - _fy23));
  newForcePlate.SetMx1(_Mx + _Fy * distZ0);
  newForcePlate.SetMy1(_My - _Fx * distZ0);

  if(_Fz != 0)
  {
    //Coordinate of the force application point (C.O.P.)
    newForcePlate.SetCOPx(-_My1/_Fz);
    newForcePlate.SetCOPy(_Mx1/_Fz);

    //Coefficients of friction
    newForcePlate.SetCOFx(_Fx/_Fz);
    newForcePlate.SetCOFy(_Fy/_Fz);
    newForcePlate.SetCOFxy(sqrt(_COFx*_COFx + _COFy*_COFy));
  }
}

void forcePlate::AnalyzeForcePlates(forcePlate &globalForcePlate, forceSensor &sensors, forcePlate &forcePlate1, forcePlate &forcePlate2, forcePlate &forcePlate3, forcePlate &forcePlate4)
//---------------------------------------------------------------------------------------
//Function: ForcePlate::AnalyzeForcePlates
//Global coordinate system (treat multiple force plates as one)
//CentorOfPressure calculation
//Reference: Kistler force plate formulae PDF
//---------------------------------------------------------------------------------------
{
  /*********FORCE PLATES MAP ********** y ********* */
  /* FRONT LEFT           FRONT RIGHT   |           */
  /* ForcePlate3         ForcePlate4    |           */
  /* ForcePlate2         ForcePlate1    |------->x  */
  /**************************************************/

  int dax1 = forcePlate1.distX;        //Force plate 1 : bottom-right quadrant
  int day1 = -forcePlate1.distY;        //(x, -y)
  int dax2 = -forcePlate2.distX;             //Force plate 2 : bottom-left quadrant
  int day2 = -forcePlate2.distY;        //(0, -y)
  int dax3 = -forcePlate3.distX;             //Force plate 3 : top-left quadrant
  int day3 = forcePlate3.distY;             //(0, 0)
  int dax4 = forcePlate4.distX;        //Force plate 4 : top-right quadrant
  int day4 = forcePlate4.distY;             //(-x, 0)

  //Calculated parameters
  globalForcePlate.SetFx(forcePlate1.GetFx() + forcePlate2.GetFx() + forcePlate3.GetFx() + forcePlate4.GetFx());
  globalForcePlate.SetFy(forcePlate1.GetFy() + forcePlate2.GetFy() + forcePlate3.GetFy() + forcePlate4.GetFy());
  globalForcePlate.SetFz(forcePlate1.GetFz() + forcePlate2.GetFz() + forcePlate3.GetFz() + forcePlate4.GetFz());

  globalForcePlate.SetMx((day1 + forcePlate1.GetCOPy())*forcePlate1.GetFz() + (day2 + forcePlate2.GetCOPy())*forcePlate2.GetFz() + (day3 + forcePlate3.GetCOPy())*forcePlate3.GetFz() + (day4 + forcePlate4.GetCOPy())*forcePlate4.GetFz());
  globalForcePlate.SetMy(-(dax1 + forcePlate1.GetCOPx())*forcePlate1.GetFz() - (dax2 + forcePlate2.GetCOPx())*forcePlate2.GetFz() - (dax3 + forcePlate3.GetCOPx())*forcePlate3.GetFz() - (dax4 + forcePlate4.GetCOPx())*forcePlate4.GetFz());

  globalForcePlate.SetMx1(_Mx + distZ0 * forcePlate1.GetFy() + distZ0 * forcePlate2.GetFy() + distZ0 * forcePlate3.GetFy() + distZ0 * forcePlate4.GetFy());
  globalForcePlate.SetMy1(_My - distZ0 * forcePlate1.GetFx() - distZ0 * forcePlate2.GetFx() - distZ0 * forcePlate3.GetFx() - distZ0 * forcePlate4.GetFx());

  if(_Fz != 0)
  {
    //Coordinate of the force application point (C.O.P.)
    globalForcePlate.SetCOPx(-globalForcePlate.GetMy1()/globalForcePlate.GetFz());  //X-Coordinate of the global application point
    globalForcePlate.SetCOPy(globalForcePlate.GetMx1()/globalForcePlate.GetFz());   //Y-Coordinate of the global application point
  }
}

//float shearing_detection1()  {
//
////Shearing detection variables
//  float left_sensors = 0, right_sensors = 0, front_sensors = 0, rear_sensors = 0;
//  float center_LR_sensors = 0, center_FR_sensors = 0;
//  left_shearing = false;
//  right_shearing = false;
//  front_shearing = false;
//  rear_shearing = false;
//
////Left-Right values
//  center_LR_sensors = max11611Data[1] + max11611Data[4] + max11611Data[7];
//  left_sensors = max11611Data[2] + max11611Data[5] + max11611Data[8];
//  right_sensors = max11611Data[9] + max11611Data[3] + max11611Data[6];
//
////Front-Rear values
//  center_FR_sensors = max11611Data[5] + max11611Data[4] + max11611Data[3];
//  front_sensors = max11611Data[2] + max11611Data[1] + max11611Data[9];
//  rear_sensors = max11611Data[8] + max11611Data[7] + max11611Data[6];
//
////Left-Right (LR) shearing detection
//  if (isUserDetected()) {
//    if (left_sensors > (right_sensors + 0.05*right_sensors)) {left_shearing = true;}
//    else if (right_sensors > (left_sensors + 0.05*left_sensors)) {right_shearing = true;}
//
//  //Front-Rear (FR) shearing detection
//    if (front_sensors > (rear_sensors + 0.05*rear_sensors)) {front_shearing = true;}
//    else if (rear_sensors > (front_sensors + 0.05*front_sensors)) {rear_shearing = true;}
//  }
//}
