#ifndef _FORCEPLATE_H_
#define _FORCEPLATE_H_

#include "forceSensor.h"

class forcePlate
{
  public:
    forcePlate();
    ~forcePlate();

    void DetectCenterOfPressure(forcePlate &globalForcePlate, forceSensor &sensors);
    void CreateForcePlate(forcePlate &newForcePlate, forceSensor &sensors, int sensorNo1, int sensorNo2, int sensorNo3, int sensorNo4);
    void AnalyzeForcePlates(forcePlate &globalForcePlate, forceSensor &sensors, forcePlate &forcePlate1, forcePlate &forcePlate2, forcePlate &forcePlate3, forcePlate &forcePlate4);

    float GetFx12() { return _fx12; }
    float GetFx34() { return _fx12; }
    float GetFy14() { return _fx12; }
    float GetFy23() { return _fx12; }
    float GetFz1() { return _fx12; }
    float GetFz2() { return _fx12; }
    float GetFz3() { return _fx12; }
    float GetFz4() { return _fx12; }
    float GetFx() { return _Fx; }
    float GetFy() { return _Fy; }
    float GetFz() { return _Fz; }
    float GetMx() { return _Mx; }
    float GetMy() { return _My; }
    float GetMz() { return _Mz; }
    float GetMx1() { return _Mx1; }
    float GetMy1() { return _My1; }
    float GetCOPx() { return _COPx; }
    float GetCOPy() { return _COPy; }
    float GetFp1COPx() { return _fp1COPx; }
    float GetFp1COPy() { return _fp1COPy; }
    float GetFp2COPx() { return _fp2COPx; }
    float GetFp2COPy() { return _fp2COPy; }
    float GetFp3COPx() { return _fp3COPx; }
    float GetFp3COPy() { return _fp3COPy; }
    float GetFp4COPx() { return _fp4COPx; }
    float GetFp4COPy() { return _fp4COPy; }
    float GetCOFx() { return _COFx; }
    float GetCOFy() { return _COFy; }
    float GetCOFxy() { return _COFxy; }

    void SetFx12(float fx12) { _fx12 = fx12; }
    void SetFx34(float fx34) { _fx34 = fx34; }
    void SetFy14(float fy14) { _fy14 = fy14; }
    void SetFy23(float fy23) { _fy23 = fy23; }
    void SetFz1(float fz1) { _fz1 = fz1; }
    void SetFz2(float fz2) { _fz2 = fz2; }
    void SetFz3(float fz3) { _fz3 = fz3; }
    void SetFz4(float fz4) { _fz4 = fz4; }
    void SetFx(float Fx) { _Fx = Fx; }
    void SetFy(float Fy) { _Fy = Fy; }
    void SetFz(float Fz) { _Fz = Fz; }
    void SetMx(float Mx) { _Mx = Mx; }
    void SetMy(float My) { _My = My; }
    void SetMz(float Mz) { _Mz = Mz; }
    void SetMx1(float Mx1) { _Mx1 = Mx1; }
    void SetMy1(float My1) { _My1 = My1; }
    void SetCOPx(float COPx) { _COPx = COPx; }
    void SetCOPy(float COPy) { _COPy = COPy; }
    void SetFp1COPx(float fp1COPx) { _fp1COPx = fp1COPx; }
    void SetFp1COPy(float fp1COPy) { _fp1COPy = fp1COPy; }
    void SetFp2COPx(float fp2COPx) { _fp2COPx = fp2COPx; }
    void SetFp2COPy(float fp2COPy) { _fp2COPy = fp2COPy; }
    void SetFp3COPx(float fp3COPx) { _fp3COPx = fp3COPx; }
    void SetFp3COPy(float fp3COPy) { _fp3COPy = fp3COPy; }
    void SetFp4COPx(float fp4COPx) { _fp4COPx = fp4COPx; }
    void SetFp4COPy(float fp4COPy) { _fp4COPy = fp4COPy; }
    void SetCOFx(float COFx) { _COFx = COFx; }
    void SetCOFy(float COFy) { _COFy = COFy; }
    void SetCOFxy(float COFxy) { _COFxy = COFxy; }

    //Constants - physical montage values
    const float distX = 4.0/2;      //Distance along X axis from SensorNo1 to SensorNo2
    const float distY = 7.5/2;      //Distance along Y axis from SensorNo2 to SensorNo3
    const float distZ0 = 0.001;    //Half of the force plate height : 0.5cm approximate for plexiglass? VALIDATE

  private:
    //Force plate output signals
    float _fx12;   //Force in X-Direction measured by SensorNo1 + SensorNo2
    float _fx34;   //Force in X-Direction measured by SensorNo3 + SensorNo4
    float _fy14;   //Force in Y-Direction measured by SensorNo1 + SensorNo4
    float _fy23;   //Force in Y-Direction measured by SensorNo2 + SensorNo3
    float _fz1;    //
    float _fz2;    //Forces in Z-Direction
    float _fz3;    //measured by SensorNo1, ..., SensorNo4
    float _fz4;    //

    //Calculated parameters
    float _Fx;     //Medio-lateral force
    float _Fy;     //Anterior-posterior force
    float _Fz;     //Vertical force
    float _Mx;     //Plate Moment about X-Axis
    float _My;     //Plate Moment about Y-Axis
    float _Mz;     //Plate Moment about Z-Axis
    float _Mx1;    //Plate Moment along X-Axis about top plate surface
    float _My1;    //Plate Moment along Y-axis about top plate surface

    //Coordinate of the force application point (C.O.P.)
    float _COPx;    //X-Coordinate of the application point
    float _COPy;    //Y-Coordinate of the application point

    //Global ForcePlates Coordinate of the force application point (C.O.P.)
    float _fp1COPx;    //X-Coordinate of the application point
    float _fp1COPy;    //Y-Coordinate of the application point
    float _fp2COPx;    //X-Coordinate of the application point
    float _fp2COPy;    //Y-Coordinate of the application point
    float _fp3COPx;    //X-Coordinate of the application point
    float _fp3COPy;    //Y-Coordinate of the application point
    float _fp4COPx;    //X-Coordinate of the application point
    float _fp4COPy;    //Y-Coordinate of the application point

    //Coefficients of friction
    float _COFx;    //Coefficient of friction X-Component
    float _COFy;    //Coefficient of friciton Y-Component
    float _COFxy;   //Coefficient of friction absolute
};


#endif /* _FORCEPLATE_H_ */
