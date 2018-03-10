//---------------------------------------------------------------------------------------
// HEADER DE FICHIER
// Description
//---------------------------------------------------------------------------------------

#include "MAX11611.h"        //10-Bit ADC
#include "ForceSensor.h"
#include "ForcePlate.h"

//External functions and variables
//Variables
extern MAX11611 max11611;                       //Initialisation of the 10-bit ADC
extern uint16_t max11611Data[sensorCount];     //Data table of size=total sensors
extern long sensedPresence;
extern long detectionThreshold;

void ForcePlate::CreateForcePlate(ForcePlate &newForcePlate, ForceSensor &sensors, int sensorNo1, int sensorNo2, int sensorNo3, int sensorNo4)
//---------------------------------------------------------------------------------------
//Function: ForcePlate::CreateForcePlate
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
  newForcePlate.setFx12(sensors.getAnalogData(sensorNo1) + sensors.getAnalogData(sensorNo2));
  newForcePlate.setFx34(sensors.getAnalogData(sensorNo3) + sensors.getAnalogData(sensorNo4));
  newForcePlate.setFy14(sensors.getAnalogData(sensorNo1) + sensors.getAnalogData(sensorNo4));
  newForcePlate.setFy23(sensors.getAnalogData(sensorNo2) + sensors.getAnalogData(sensorNo3));
  newForcePlate.setFz1(sensors.getAnalogData(sensorNo1));
  newForcePlate.setFz2(sensors.getAnalogData(sensorNo2));
  newForcePlate.setFz3(sensors.getAnalogData(sensorNo3));
  newForcePlate.setFz4(sensors.getAnalogData(sensorNo4));

  //Calculated parameters
  newForcePlate.setFx(_fx12 + _fx34);
  newForcePlate.setFy(_fy14 + _fy23);
  newForcePlate.setFz(_fz1 + _fz2 + _fz3 + _fz4);
  newForcePlate.setMx(distY * (_fz1 + _fz2 - _fz3 - _fz4));
  newForcePlate.setMy(distX * (-_fz1 + _fz2 + _fz3 - _fz4));
  newForcePlate.setMz(distY * (-_fx12 + _fx34) + distX * (_fy14 - _fy23));
  newForcePlate.setMx1(_Mx + _Fy * distZ0);
  newForcePlate.setMy1(_My - _Fx * distZ0);

  //Coordinate of the force application point (C.O.P.)
  newForcePlate.setCOPx(-_My1/_Fz);
  newForcePlate.setCOPy(_Mx1/_Fz);

  //Coefficients of friction
  newForcePlate.setCOFx(_Fx/_Fz);
  newForcePlate.setCOFy(_Fy/_Fz);
  newForcePlate.setCOFxy(sqrt(_COFx*_COFx + _COFy*_COFy));
}

void ForcePlate::AnalyzeForcePlates(ForcePlate &GlobalForcePlate, ForceSensor &Sensors, ForcePlate &ForcePlate1, ForcePlate &ForcePlate2, ForcePlate &ForcePlate3, ForcePlate &ForcePlate4)
//---------------------------------------------------------------------------------------
//Function: ForcePlate::AnalyzeForcePlates
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

  int dax1 = -distX;        //Force plate 1 : bottom-right quadrant
  int day1 = -distY;        //(-x, -y)
  int dax2 = 0;             //Force plate 2 : bottom-left quadrant
  int day2 = -distY;        //(0, -y)
  int dax3 = 0;             //Force plate 3 : top-left quadrant
  int day3 = 0;             //(0, 0)
  int dax4 = -distX;        //Force plate 4 : top-right quadrant
  int day4 = 0;             //(-x, 0)

  //Calculated parameters
  GlobalForcePlate.setFx(ForcePlate1.getFx() + ForcePlate2.getFx() + ForcePlate3.getFx() + ForcePlate4.getFx());
  GlobalForcePlate.setFy(ForcePlate1.getFy() + ForcePlate2.getFy() + ForcePlate3.getFy() + ForcePlate4.getFy());
  GlobalForcePlate.setFz(ForcePlate1.getFz() + ForcePlate2.getFz() + ForcePlate3.getFz() + ForcePlate4.getFz());

  GlobalForcePlate.setMx((day1 + distY)*ForcePlate1.getFz() + (day2 + distY)*ForcePlate2.getFz() + (day3 + distY)*ForcePlate3.getFz() + (day4 + distY)*ForcePlate4.getFz());
  GlobalForcePlate.setMy(-(dax1 + distX)*ForcePlate1.getFz() - (dax2 + distX)*ForcePlate2.getFz() - (dax3 + distX)*ForcePlate3.getFz() - (dax4 + distX)*ForcePlate4.getFz());

  GlobalForcePlate.setMx1(_Mx + distZ0 * ForcePlate1.getFy() + distZ0 * ForcePlate2.getFy() + distZ0 * ForcePlate3.getFy() + distZ0 * ForcePlate4.getFy());
  GlobalForcePlate.setMy1(_My - distZ0 * ForcePlate1.getFx() - distZ0 * ForcePlate2.getFx() - distZ0 * ForcePlate3.getFx() - distZ0 * ForcePlate4.getFx());

  //Coordinate of the force application point (C.O.P.)
  GlobalForcePlate.setCOPx(-GlobalForcePlate.getMy1()/GlobalForcePlate.getFz());  //X-Coordinate of the global application point
  GlobalForcePlate.setCOPy(GlobalForcePlate.getMx1()/GlobalForcePlate.getFz());   //Y-Coordinate of the global application point
}

//double shearing_detection1()  {
//
////Shearing detection variables
//  double left_sensors = 0, right_sensors = 0, front_sensors = 0, rear_sensors = 0;
//  double center_LR_sensors = 0, center_FR_sensors = 0;
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
//  if (isPresenceDetected()) {
//    if (left_sensors > (right_sensors + 0.05*right_sensors)) {left_shearing = true;}
//    else if (right_sensors > (left_sensors + 0.05*left_sensors)) {right_shearing = true;}
//
//  //Front-Rear (FR) shearing detection
//    if (front_sensors > (rear_sensors + 0.05*rear_sensors)) {front_shearing = true;}
//    else if (rear_sensors > (front_sensors + 0.05*front_sensors)) {rear_shearing = true;}
//  }
//}
