#include "ForcePlate.h"
#include "ForceSensor.h"

#include <stdio.h>
#include <unistd.h>

//---------------------------------------------------------------------------------------
//Force plate creation from 2x2 force sensors matrix
//Each force plate is composed of 4 sensors from input
//Reference: Kistler force plate formulae PDF
//---------------------------------------------------------------------------------------
ForcePlate::ForcePlate(ForceSensor &sensors, uint8_t sensorNo1, uint8_t sensorNo2, uint8_t sensorNo3, uint8_t sensorNo4, float distX, float distY, float distZ)
{
    /********FORCE PLATE MAP ******** y ******** ***/
    /* FRONT LEFT       FRONT RIGHT   |           */
    /* SensorNo3         SensorNo4    |           */
    /* SensorNo2         SensorNo1    |------->x  */
    /**********************************************/
    _fx12 = 0.0f;
    _fx34 = 0.0f;
    _fy14 = 0.0f;
    _fy23 = 0.0f;
    _fz1 = 0.0f;
    _fz2 = 0.0f;
    _fz3 = 0.0f;
    _fz4 = 0.0f;
    _fx = 0.0f;
    _fy = 0.0f;
    _fz = 0.0f;
    _mx = 0.0f;
    _my = 0.0f;
    _mz = 0.0f;
    _mx1 = 0.0f;
    _my1 = 0.0f;

    _centerOfPressure = {0.0f, 0.0f};

    _forceSensor = &sensors;

    _distX = distX;
    _distY = distY;
    _distZ = _distZ;

    _sensorNo1 = sensorNo1;
    _sensorNo2 = sensorNo2;
    _sensorNo3 = sensorNo3;
    _sensorNo4 = sensorNo4;

    Update();
}

void ForcePlate::Update()
{
    //Force plate output signals
    _fx12 = static_cast<float>(_forceSensor->GetAnalogData(_sensorNo1) + _forceSensor->GetAnalogData(_sensorNo2));
    _fx34 = static_cast<float>(_forceSensor->GetAnalogData(_sensorNo3) + _forceSensor->GetAnalogData(_sensorNo4));
    _fy14 = static_cast<float>(_forceSensor->GetAnalogData(_sensorNo1) + _forceSensor->GetAnalogData(_sensorNo4));
    _fy23 = static_cast<float>(_forceSensor->GetAnalogData(_sensorNo2) + _forceSensor->GetAnalogData(_sensorNo3));
    _fz1 = static_cast<float>(_forceSensor->GetAnalogData(_sensorNo1));
    _fz2 = static_cast<float>(_forceSensor->GetAnalogData(_sensorNo2));
    _fz3 = static_cast<float>(_forceSensor->GetAnalogData(_sensorNo3));
    _fz4 = static_cast<float>(_forceSensor->GetAnalogData(_sensorNo4));

    //Calculated parameters
    _fx = _fx12 + _fx34;
    _fy = _fy14 + _fy23;
    _fz = _fz1 + _fz2 + _fz3 + _fz4;
    _mx = -_distY * (_fz1 + _fz2 - _fz3 - _fz4);
    _my = _distX * (-_fz1 + _fz2 + _fz3 - _fz4);
    _mz = -_distY * (-_fx12 + _fx34) + _distX * (_fy14 - _fy23);
    _mx1 = _mx + (_fy * _distZ);
    _my1 = _my - (_fx * _distZ);

    if (_fz != 0)
    {
        //Coordinate of the force application point (C.O.P.)
        _centerOfPressure = {(-_my1 / _fz), (_mx1 / _fz)};
    }
}