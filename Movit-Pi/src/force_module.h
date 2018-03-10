#ifndef _FORCE_MODULE_H_
#define _FORCE_MODULE_H_

bool isSomeoneThere();
void ForceSensorUnits();
void shearing_detection1();

class ForcePlate
{
  public:
    //Functions
    void CreateForcePlate(ForcePlate &NewForcePlate, int SensorNo1, int SensorNo2, int SensorNo3, int SensorNo4);
    void AnalyzeForcePlates(ForcePlate &GlobalForcePlate, ForcePlate &ForcePlate1, ForcePlate &ForcePlate2, ForcePlate &ForcePlate3, ForcePlate &ForcePlate4);

    //Variables
    //Force plate output signals
    int fx12; //Force in X-Direction measured by SensorNo1 + SensorNo2
    int fx34; //Force in X-Direction measured by SensorNo3 + SensorNo4
    int fy14; //Force in Y-Direction measured by SensorNo1 + SensorNo4
    int fy23; //Force in Y-Direction measured by SensorNo2 + SensorNo3
    int fz1;  //
    int fz2;  //Forces in Z-Direction
    int fz3;  //measured by SensorNo1, ..., SensorNo4
    int fz4;  //

    //Force sensors positioning
    int a;   //Distance along X axis from SensorNo1 to SensorNo2
    int b;   //Distance along Y axis from SensorNo2 to SensorNo3
    int az0; //Half of the force plate height : 0.5cm approximate for plexiglass? VALIDATE

    //Calculated parameters
    int Fx;  //Medio-lateral force
    int Fy;  //Anterior-posterior force
    int Fz;  //Vertical force
    int Mx;  //Plate Moment about X-Axis
    int My;  //Plate Moment about Y-Axis
    int Mz;  //Plate Moment about Z-Axis
    int Mx1; //Plate Moment along X-Axis about top plate surface
    int My1; //Plate Moment along Y-axis about top plate surface

    //Coordinate of the force application point (C.O.P.)
    int COPx; //X-Coordinate of the application point
    int COPy; //Y-Coordinate of the application point

    //Coefficients of friction
    int COFx;  //Coefficient of friction X-Component
    int COFy;  //Coefficient of friciton Y-Component
    int COFxy; //Coefficient of friction absolute
};

#endif /* _FORCE_MODULE_H_ */
