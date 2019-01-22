#include "PressureMat.h"

PressureMat::PressureMat() : _forcePlate1(_sensorMatrix, 4, 1, 0, 3, _distX, _distY, _distZ0),
                             _forcePlate2(_sensorMatrix, 7, 4, 3, 6, _distX, _distY, _distZ0),
                             _forcePlate3(_sensorMatrix, 5, 2, 1, 4, _distX, _distY, _distZ0),
                             _forcePlate4(_sensorMatrix, 8, 5, 4, 7, _distX, _distY, _distZ0)
{
}

bool PressureMat::Initialize()
{
    _isForcePlateInitialized = InitializeForcePlate();

    if (!_isForcePlateInitialized)
    {
        return false;
    }

    pressure_mat_offset_t pressureMatOffset = _sensorMatrix.GetOffsets();
    if (IsPressureMatOffsetValid(pressureMatOffset))
    {
        _isCalibrated = true;
    }
    return true;
}

bool PressureMat::IsConnected()
{
    return _max11611.Initialize();
}

bool PressureMat::InitializeForcePlate()
{
    printf("MAX11611 (ADC) initializing ... ");
    if (IsConnected())
    {
        for (uint8_t i = 0; i < PRESSURE_SENSOR_COUNT; i++)
        {
            _sensorMatrix.SetAnalogData(0, i);
        }

        printf("success\n");
        return true;
    }
    else
    {
        printf("FAIL\n");
        return false;
    }
}

void PressureMat::Calibrate()
{
    const uint8_t maxIterations = 10; //Calibration preceision adjustement - Number of measures (1s) in final mean
    UpdateForcePlateData();
    _sensorMatrix.CalibrateForceSensor(_max11611, _max11611Data, maxIterations);
    _isCalibrated = true;
}

void PressureMat::Update()
{
    if (_isForcePlateInitialized && _isCalibrated)
    {
        // Data: Capteur de force
        UpdateForcePlateData();

        _isSomeoneThere = _sensorMatrix.IsUserDetected();
        if (_isSomeoneThere)
        {
            DetectCenterOfPressure();

            _pressureMatData.quadrantPressure[GlobalForcePlate::Quadrant::FrontLeft] = _forcePlate1.GetCenterOfPressure();
            _pressureMatData.quadrantPressure[GlobalForcePlate::Quadrant::FrontRight] = _forcePlate2.GetCenterOfPressure();
            _pressureMatData.quadrantPressure[GlobalForcePlate::Quadrant::BackLeft] = _forcePlate3.GetCenterOfPressure();
            _pressureMatData.quadrantPressure[GlobalForcePlate::Quadrant::BackRight] = _forcePlate4.GetCenterOfPressure();

            _pressureMatData.centerOfPressure = _globalForcePlate.GetCenterOfPressure();
        }
        else
        {
            for (uint8_t i = 0; i < GlobalForcePlate::Quadrant::Count; i++)
            {
                _pressureMatData.quadrantPressure[i] = {DEFAULT_CENTER_OF_PRESSURE, DEFAULT_CENTER_OF_PRESSURE};
            }
            _pressureMatData.centerOfPressure = {DEFAULT_CENTER_OF_PRESSURE, DEFAULT_CENTER_OF_PRESSURE};
        }
    }
    else
    {
        for (uint8_t i = 0; i < GlobalForcePlate::Quadrant::Count; i++)
        {
            _pressureMatData.quadrantPressure[i] = {DEFAULT_CENTER_OF_PRESSURE, DEFAULT_CENTER_OF_PRESSURE};
        }
        _pressureMatData.centerOfPressure = {DEFAULT_CENTER_OF_PRESSURE, DEFAULT_CENTER_OF_PRESSURE};
        _isSomeoneThere = false;
    }
}

void PressureMat::UpdateForcePlateData()
{
    _max11611.GetData(PRESSURE_SENSOR_COUNT, _max11611Data);
    for (uint8_t i = 0; i < PRESSURE_SENSOR_COUNT; i++)
    {
        _sensorMatrix.SetAnalogData(i, _max11611Data[i]);
    }
}

bool PressureMat::IsPressureMatOffsetValid(pressure_mat_offset_t offset)
{
    for (int i = 0; i < PRESSURE_SENSOR_COUNT; i++)
    {
        if (offset.analogOffset[i] != 0)
        {
            return true;
        }
    }
    return offset.detectionThreshold != 0.0f || offset.totalSensorMean != 0;
}

//---------------------------------------------------------------------------------------
//Function: DetectCenterOfPressure
//Global coordinate system (treat multiple force plates as one)
//Global system input is four 2x2 matrix (decomposed from a global 3x3 matrix)
//Reference: Kistler force plate formulae PDF
//---------------------------------------------------------------------------------------
void PressureMat::DetectCenterOfPressure()
{
    //Global analysis of the 4 plates (treated as one plate)
    /*********FORCE PLATES MAP ********** y ********* */
    /* FRONT LEFT           FRONT RIGHT   |           */
    /* ForcePlate1         ForcePlate2    |           */
    /* ForcePlate3         ForcePlate4    |------->x  */
    /**************************************************/

    const float dax1 = -_distX; //Force plate 1 : bottom-right quadrant
    const float day1 = _distY;  //(x, -y)
    const float dax2 = _distX;  //Force plate 2 : bottom-left quadrant
    const float day2 = _distY;  //(0, -y)
    const float dax3 = -_distX; //Force plate 3 : top-left quadrant
    const float day3 = -_distY; //(0, 0)
    const float dax4 = _distX;  //Force plate 4 : top-right quadrant
    const float day4 = -_distY; //(-x, 0)

    //Update force plate
    _forcePlate1.Update();
    _forcePlate2.Update();
    _forcePlate3.Update();
    _forcePlate4.Update();

    //Calculated parameters
    _globalForcePlate.SetFx(_forcePlate1.GetFx() + _forcePlate2.GetFx() + _forcePlate3.GetFx() + _forcePlate4.GetFx());
    _globalForcePlate.SetFy(_forcePlate1.GetFy() + _forcePlate2.GetFy() + _forcePlate3.GetFy() + _forcePlate4.GetFy());
    _globalForcePlate.SetFz(_forcePlate1.GetFz() + _forcePlate2.GetFz() + _forcePlate3.GetFz() + _forcePlate4.GetFz());

    _globalForcePlate.SetMx((day1 + _forcePlate1.GetCenterOfPressure().y) * _forcePlate1.GetFz() + (day2 + _forcePlate2.GetCenterOfPressure().y) * _forcePlate2.GetFz() + (day3 + _forcePlate3.GetCenterOfPressure().y) * _forcePlate3.GetFz() + (day4 + _forcePlate4.GetCenterOfPressure().y) * _forcePlate4.GetFz());
    _globalForcePlate.SetMy(-(dax1 + _forcePlate1.GetCenterOfPressure().x) * _forcePlate1.GetFz() - (dax2 + _forcePlate2.GetCenterOfPressure().x) * _forcePlate2.GetFz() - (dax3 + _forcePlate3.GetCenterOfPressure().x) * _forcePlate3.GetFz() - (dax4 + _forcePlate4.GetCenterOfPressure().x) * _forcePlate4.GetFz());

    _globalForcePlate.SetMx1(_globalForcePlate.GetMx() + _distZ0 * _forcePlate1.GetFy() + _distZ0 * _forcePlate2.GetFy() + _distZ0 * _forcePlate3.GetFy() + _distZ0 * _forcePlate4.GetFy());
    _globalForcePlate.SetMy1(_globalForcePlate.GetMy() - _distZ0 * _forcePlate1.GetFx() - _distZ0 * _forcePlate2.GetFx() - _distZ0 * _forcePlate3.GetFx() - _distZ0 * _forcePlate4.GetFx());

    if (_globalForcePlate.GetFx() != 0.0f)
    {
        //Coordinate of the force application point (C.O.P.)
        Coord_t centerOfPressure = {(-_globalForcePlate.GetMy1() / _globalForcePlate.GetFz()), (_globalForcePlate.GetMx1() / _globalForcePlate.GetFz())};
        _globalForcePlate.SetCenterOfPressure(centerOfPressure);
    }
}
