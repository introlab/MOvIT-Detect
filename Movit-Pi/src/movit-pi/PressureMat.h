#ifndef PRESSURE_MATH_H
#define PRESSURE_MATH_H

#include "GlobalForcePlate.h"
#include "FileManager.h"
#include "ForceSensor.h"
#include "ForcePlate.h"
#include "MAX11611.h"
#include "Sensor.h"
#include "Utils.h"

class PressureMat : public Sensor
{
  public:
	void Update();
	void Calibrate();

	bool Initialize();
	bool IsConnected();

	bool IsForcePlateConnected();
	bool IsSomeoneThere() { return _isSomeoneThere; }
	bool IsCalibrated() { return _isCalibrated; }

	pressure_mat_data_t GetPressureMatData() { return _pressureMatData; }
	pressure_mat_offset_t GetOffsets() { return _sensorMatrix.GetOffsets(); }
	void SetOffsets(pressure_mat_offset_t pressureMatOffset) { _sensorMatrix.SetOffsets(pressureMatOffset); }

	// Singleton
	static PressureMat *GetInstance()
	{
		static PressureMat instance;
		return &instance;
	}

	void DetectCenterOfPressure();
	void UpdateForcePlateData();

  private:
	//Singleton
	PressureMat();
	PressureMat(PressureMat const &);	// Don't Implement.
	void operator=(PressureMat const &); // Don't implement.

	const float DEFAULT_CENTER_OF_PRESSURE = 0.0f;

	//Constants - physical montage values
	const float _distX = 2.0f;   //Distance along X axis from SensorNo1 to SensorNo2
	const float _distY = 2.0f;   //Distance along Y axis from SensorNo2 to SensorNo3
	const float _distZ0 = 0.001; //Half of the force plate height : 0.5cm approximate for plexiglass? VALIDATE

	bool IsPressureMatOffsetValid(pressure_mat_offset_t offset);
	bool InitializeForcePlate();

	bool _isSomeoneThere = false;
	bool _isForcePlateInitialized = false;
	bool _isCalibrated = false;

	MAX11611 _max11611;
	uint16_t _max11611Data[PRESSURE_SENSOR_COUNT];

	ForceSensor _sensorMatrix;
	GlobalForcePlate _globalForcePlate;

	pressure_mat_data_t _pressureMatData;

	ForcePlate _forcePlate1;
	ForcePlate _forcePlate2;
	ForcePlate _forcePlate3;
	ForcePlate _forcePlate4;
};

#endif // PRESSURE_MATH_H
