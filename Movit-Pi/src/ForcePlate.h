#ifndef _FORCEPLATE_H_
#define _FORCEPLATE_H_

class ForcePlate
{
  public:
    void CreateForcePlate(ForcePlate &newForcePlate, ForceSensor &sensors, int sensorNo1, int sensorNo2, int sensorNo3, int sensorNo4);
    void AnalyzeForcePlates(ForcePlate &globalForcePlate, ForceSensor &sensors, ForcePlate &forcePlate1, ForcePlate &forcePlate2, ForcePlate &forcePlate3, ForcePlate &forcePlate4);

    double getFx12() { return _fx12; }
    double getFx34() { return _fx12; }
    double getFy14() { return _fx12; }
    double getFy23() { return _fx12; }
    double getFz1() { return _fx12; }
    double getFz2() { return _fx12; }
    double getFz3() { return _fx12; }
    double getFz4() { return _fx12; }
    double getFx() { return _Fx; }
    double getFy() { return _Fy; }
    double getFz() { return _Fz; }
    double getMx() { return _Mx; }
    double getMy() { return _My; }
    double getMz() { return _Mz; }
    double getMx1() { return _Mx1; }
    double getMy1() { return _My1; }
    double getCOPx() { return _COPx; }
    double getCOPy() { return _COPy; }
    double getCOFx() { return _COFx; }
    double getCOFy() { return _COFy; }
    double getCOFxy() { return _COFxy; }

    void setFx12(double fx12) { _fx12 = fx12; }
    void setFx34(double fx34) { _fx34 = fx34; }
    void setFy14(double fy14) { _fy14 = fy14; }
    void setFy23(double fy23) { _fy23 = fy23; }
    void setFz1(double fz1) { _fz1 = fz1; }
    void setFz2(double fz2) { _fz2 = fz2; }
    void setFz3(double fz3) { _fz3 = fz3; }
    void setFz4(double fz4) { _fz4 = fz4; }
    void setFx(double Fx) { _Fx = Fx; }
    void setFy(double Fy) { _Fy = Fy; }
    void setFz(double Fz) { _Fz = Fz; }
    void setMx(double Mx) { _Mx = Mx; }
    void setMy(double My) { _My = My; }
    void setMz(double Mz) { _Mz = Mz; }
    void setMx1(double Mx1) { _Mx1 = Mx1; }
    void setMy1(double My1) { _My1 = My1; }
    void setCOPx(double COPx) { _COPx = COPx; }
    void setCOPy(double COPy) { _COPy = COPy; }
    void setCOFx(double COFx) { _COFx = COFx; }
    void setCOFy(double COFy) { _COFy = COFy; }
    void setCOFxy(double COFxy) { _COFxy = COFxy; }

    //Constants - physical montage values
    const double distX = 0.04;   //Distance along X axis from SensorNo1 to SensorNo2
    const double distY = 0.075;  //Distance along Y axis from SensorNo2 to SensorNo3
    const double distZ0 = 0.005; //Half of the force plate height : 0.5cm approximate for plexiglass? VALIDATE

  private:
    //Force plate output signals
    double _fx12; //Force in X-Direction measured by SensorNo1 + SensorNo2
    double _fx34; //Force in X-Direction measured by SensorNo3 + SensorNo4
    double _fy14; //Force in Y-Direction measured by SensorNo1 + SensorNo4
    double _fy23; //Force in Y-Direction measured by SensorNo2 + SensorNo3
    double _fz1;  //
    double _fz2;  //Forces in Z-Direction
    double _fz3;  //measured by SensorNo1, ..., SensorNo4
    double _fz4;  //

    //Calculated parameters
    double _Fx;  //Medio-lateral force
    double _Fy;  //Anterior-posterior force
    double _Fz;  //Vertical force
    double _Mx;  //Plate Moment about X-Axis
    double _My;  //Plate Moment about Y-Axis
    double _Mz;  //Plate Moment about Z-Axis
    double _Mx1; //Plate Moment along X-Axis about top plate surface
    double _My1; //Plate Moment along Y-axis about top plate surface

    //Coordinate of the force application point (C.O.P.)
    double _COPx; //X-Coordinate of the application point
    double _COPy; //Y-Coordinate of the application point

    //Coefficients of friction
    double _COFx;  //Coefficient of friction X-Component
    double _COFy;  //Coefficient of friciton Y-Component
    double _COFxy; //Coefficient of friction absolute
};

#endif /* _FORCEPLATE_H_ */
