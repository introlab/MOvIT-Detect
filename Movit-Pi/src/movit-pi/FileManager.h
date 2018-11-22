#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include "Utils.h"
#include "DataType.h"

#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/document.h"

class FileManager
{
  public:
	void Read();
	void Save();

	notifications_settings_t GetNotificationsSettings() { return _notificationsSettings; }
	pressure_mat_offset_t GetPressureMatoffset() { return _pressureMatOffset; }
	tilt_settings_t GetTiltSettings() { return _tiltSettings; }
	imu_offset_t GetMobileImuOffsets();
	imu_offset_t GetFixedImuOffsets();

	void SetPressureMatOffsets(pressure_mat_offset_t offset) { _pressureMatOffset = offset; }
	void SetTiltSettings(tilt_settings_t tiltSettings) { _tiltSettings = tiltSettings; }
	void SetMobileImuOffsets(imu_offset_t offset) { _mobileImuOffset = offset; }
	void SetFixedImuOffsets(imu_offset_t offset) { _fixedImuOffset = offset; }
	void SetNotificationsSettings(notifications_settings_t notificationsSettings) { _notificationsSettings = notificationsSettings; }

	// Singleton
	static FileManager *GetInstance()
	{
		static FileManager instance;
		return &instance;
	}

  private:
	//Singleton
	FileManager() = default;
	FileManager(FileManager const &);	// Don't Implement.
	void operator=(FileManager const &); // Don't implement.

	notifications_settings_t _notificationsSettings;
	pressure_mat_offset_t _pressureMatOffset;
	tilt_settings_t _tiltSettings;
	imu_offset_t _mobileImuOffset;
	imu_offset_t _fixedImuOffset;

	void FormatNotificationsSettings(rapidjson::Writer<rapidjson::StringBuffer> &writer, notifications_settings_t notificationsSettings, std::string objectName);
	void FormatPressureMatOffset(rapidjson::Writer<rapidjson::StringBuffer> &writer, pressure_mat_offset_t offset, std::string objectName);
	void FormatTiltSettings(rapidjson::Writer<rapidjson::StringBuffer> &writer, tilt_settings_t tiltSettings, std::string objectName);
	void FormatImuOffset(rapidjson::Writer<rapidjson::StringBuffer> &writer, imu_offset_t offset, std::string objectName);

	imu_offset_t ParseIMUOffset(rapidjson::Document &document, std::string objectName);
	notifications_settings_t ParseNotificationsSettings(rapidjson::Document &document);
	pressure_mat_offset_t ParsePressureMatOffset(rapidjson::Document &document);
	tilt_settings_t ParseTiltSettings(rapidjson::Document &document);
};

#endif // FILE_MANAGER_H
