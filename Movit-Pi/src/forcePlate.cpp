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

forcePlate::forcePlate()
{
  _fx12 = 0;
  _fx34 = 0;
  _fy14 = 0;
  _fy23 = 0;
  _fz1 = 0;
  _fz2 = 0;
  _fz3 = 0;
  _fz4 = 0;
  _Fx = 0;
  _Fy = 0;
  _Fz = 0;
  _Mx = 0;
  _My = 0;
  _Mz = 0;
  _Mx1 = 0;
  _My1 = 0;
  _COPx = 0;
  _COPy = 0;
  _fp1COPx = 0;
  _fp1COPy = 0;
  _fp2COPx = 0;
  _fp2COPy = 0;
  _fp3COPx = 0;
  _fp3COPy = 0;
  _fp4COPx = 0;
  _fp4COPy = 0;
  _COFx = 0;
  _COFy = 0;
  _COFxy = 0;
}

forcePlate::~forcePlate() {}

void forcePlate::DetectCenterOfPressure(forcePlate &globalForcePlate, forceSensor &sensors)
//---------------------------------------------------------------------------------------
//Function: ForcePlate::DetectCenterOfPressure
//Centor of Pressure calculation for each quadrant and global system
//Reference: Kistler force plate formulae PDF
//---------------------------------------------------------------------------------------
{
  /*********FORCE PLATES MAP ********** y ********* */
  /* FRONT LEFT           FRONT RIGHT   |           */
  /* ForcePlate1         ForcePlate2    |           */
  /* ForcePlate3         ForcePlate4    |------->x  */
  /**************************************************/

  //Force plates variables
  forcePlate forcePlate1;
  forcePlate forcePlate2;
  forcePlate forcePlate3;
  forcePlate forcePlate4;

  //Creation of the 4 ForcePlates
  forcePlate1.CreateForcePlate(forcePlate1, sensors, 4, 1, 0, 3);
  forcePlate2.CreateForcePlate(forcePlate2, sensors, 7, 4, 3, 6);
  forcePlate3.CreateForcePlate(forcePlate3, sensors, 5, 2, 1, 4);
  forcePlate4.CreateForcePlate(forcePlate4, sensors, 8, 5, 4, 7);

  //Global analysis of the 4 plates (treated as one plate)
  globalForcePlate.AnalyzeForcePlates(globalForcePlate, sensors, forcePlate1, forcePlate2, forcePlate3, forcePlate4);
  globalForcePlate.SetFp1COPx(forcePlate1.GetCOPx());
  globalForcePlate.SetFp1COPy(forcePlate1.GetCOPy());
  globalForcePlate.SetFp2COPx(forcePlate2.GetCOPx());
  globalForcePlate.SetFp2COPy(forcePlate2.GetCOPy());
  globalForcePlate.SetFp3COPx(forcePlate3.GetCOPx());
  globalForcePlate.SetFp3COPy(forcePlate3.GetCOPy());
  globalForcePlate.SetFp4COPx(forcePlate4.GetCOPx());
  globalForcePlate.SetFp4COPy(forcePlate4.GetCOPy());
}

void forcePlate::CreateForcePlate(forcePlate &newForcePlate, forceSensor &sensors, int sensorNo1, int sensorNo2, int sensorNo3, int sensorNo4)
//---------------------------------------------------------------------------------------
//Function: ForcePlate::CreateForcePlate
//Force plate creation from 2x2 force sensors matrix
//Each force plate is composed of 4 sensors from input
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
//Global system input is four 2x2 matrix (decomposed from a global 3x3 matrix)
//Reference: Kistler force plate formulae PDF
//---------------------------------------------------------------------------------------
{
  /*********FORCE PLATES MAP ********** y ********* */
  /* FRONT LEFT           FRONT RIGHT   |           */
  /* ForcePlate1         ForcePlate2    |           */
  /* ForcePlate3         ForcePlate4    |------->x  */
  /**************************************************/

  int dax1 = -forcePlate1.distX;       //Force plate 1 : bottom-right quadrant
  int day1 = forcePlate1.distY;        //(x, -y)
  int dax2 = forcePlate2.distX;        //Force plate 2 : bottom-left quadrant
  int day2 = forcePlate2.distY;        //(0, -y)
  int dax3 = -forcePlate3.distX;       //Force plate 3 : top-left quadrant
  int day3 = -forcePlate3.distY;       //(0, 0)
  int dax4 = forcePlate4.distX;        //Force plate 4 : top-right quadrant
  int day4 = -forcePlate4.distY;       //(-x, 0)

  //Calculated parameters
  globalForcePlate.SetFx(forcePlate1.GetFx() + forcePlate2.GetFx() + forcePlate3.GetFx() + forcePlate4.GetFx());
  globalForcePlate.SetFy(forcePlate1.GetFy() + forcePlate2.GetFy() + forcePlate3.GetFy() + forcePlate4.GetFy());
  globalForcePlate.SetFz(forcePlate1.GetFz() + forcePlate2.GetFz() + forcePlate3.GetFz() + forcePlate4.GetFz());

  printf("\nGlob. Fx : %f\n", globalForcePlate.GetFx());
  printf("Glob. Fy : %f\n", globalForcePlate.GetFy());
  printf("Glob. Fz : %f\n", globalForcePlate.GetFz());

  globalForcePlate.SetMx((day1 + forcePlate1.GetCOPy())*forcePlate1.GetFz() + (day2 + forcePlate2.GetCOPy())*forcePlate2.GetFz() + (day3 + forcePlate3.GetCOPy())*forcePlate3.GetFz() + (day4 + forcePlate4.GetCOPy())*forcePlate4.GetFz());
  globalForcePlate.SetMy(-(dax1 + forcePlate1.GetCOPx())*forcePlate1.GetFz() - (dax2 + forcePlate2.GetCOPx())*forcePlate2.GetFz() - (dax3 + forcePlate3.GetCOPx())*forcePlate3.GetFz() - (dax4 + forcePlate4.GetCOPx())*forcePlate4.GetFz());

  printf("\nGlob. Mx : %f\n", globalForcePlate.GetMx());
  printf("Glob. My : %f\n", globalForcePlate.GetMy());

  globalForcePlate.SetMx1(_Mx + distZ0 * forcePlate1.GetFy() + distZ0 * forcePlate2.GetFy() + distZ0 * forcePlate3.GetFy() + distZ0 * forcePlate4.GetFy());
  globalForcePlate.SetMy1(_My - distZ0 * forcePlate1.GetFx() - distZ0 * forcePlate2.GetFx() - distZ0 * forcePlate3.GetFx() - distZ0 * forcePlate4.GetFx());

  if(_Fz != 0)
  {
    //Coordinate of the force application point (C.O.P.)
    globalForcePlate.SetCOPx(-globalForcePlate.GetMy1()/globalForcePlate.GetFz());  //X-Coordinate of the global application point
    globalForcePlate.SetCOPy(globalForcePlate.GetMx1()/globalForcePlate.GetFz());   //Y-Coordinate of the global application point
  }
}
