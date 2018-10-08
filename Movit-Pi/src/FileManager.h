#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include "Utils.h"

#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/document.h"

class FileManager
{
public:
  void Read();
  void Save();

  imu_offset_t GetMobileImuOffsets();
  imu_offset_t GetFixedImuOffsets();
  pressure_mat_offset_t GetPressureMatoffset() { return _pressureMatOffset; }

  void SetMobileImuOffsets(imu_offset_t offset) { _mobileImuOffset = offset; }
  void SetFixedImuOffsets(imu_offset_t offset) { _fixedImuOffset = offset; }
  void SetPressureMatOffsets(pressure_mat_offset_t offset) { _pressureMatOffset = offset; }

  // Singleton
  static FileManager *GetInstance()
  {
    static FileManager instance;
    return &instance;
  }

private:
  //Singleton
  FileManager() = default;
  FileManager(FileManager const &);    // Don't Implement.
  void operator=(FileManager const &); // Don't implement.

  pressure_mat_offset_t _pressureMatOffset;
  imu_offset_t _fixedImuOffset;
  imu_offset_t _mobileImuOffset;

  void FormatPressureMatOffset(rapidjson::Writer<rapidjson::StringBuffer> &writer, pressure_mat_offset_t offset, std::string objectName);
  void FormatImuOffset(rapidjson::Writer<rapidjson::StringBuffer> &writer, imu_offset_t offset, std::string objectName);

  imu_offset_t ParseIMUOffset(rapidjson::Document &document, std::string objectName);
  pressure_mat_offset_t ParsePressureMatOffset(rapidjson::Document &document);
};

#endif // FILE_MANAGER_H
