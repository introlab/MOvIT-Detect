#include "FileManager.h"
#include "Utils.h"
#include "rapidjson/document.h"
#include <iostream>
#include <fstream>

using rapidjson::Document;
using rapidjson::StringBuffer;
using rapidjson::Value;
using rapidjson::Writer;
using std::string;

const std::string OFFSETS_FILENAME = "offsets.txt";

const std::string PRESSURE_MAT_OBJECT = "pressure_mat_offset";
const std::string FIXED_IMU_OBJECT = "fixed_imu_offset";
const std::string MOBILE_IMU_OBJECT = "mobile_imu_offset";

void FileManager::Read()
{
    std::fstream file;
    file.open(OFFSETS_FILENAME, std::fstream::in | std::fstream::out);

    if (!file.is_open())
    {
        printf("ReadFile: error opening %s...\n", OFFSETS_FILENAME.c_str());
        return;
    }

    std::string line;
    while (getline(file, line))
    {
        Document doc;
        doc.Parse(line.c_str());

        if (doc.IsObject())
        {
            _pressureMatOffset = ParsePressureMatOffset(doc);
            _fixedImuOffset = ParseIMUOffset(doc, FIXED_IMU_OBJECT);
            _mobileImuOffset = ParseIMUOffset(doc, MOBILE_IMU_OBJECT);
        }
    }
    file.close();
}

void FileManager::Save()
{
    StringBuffer strBuff;
    Writer<StringBuffer> writer(strBuff);

    writer.StartObject();
    FormatPressureMatOffset(writer, _pressureMatOffset, PRESSURE_MAT_OBJECT);
    FormatImuOffset(writer, _fixedImuOffset, FIXED_IMU_OBJECT);
    FormatImuOffset(writer, _mobileImuOffset, MOBILE_IMU_OBJECT);
    writer.EndObject();

    std::fstream file;
    file.open(OFFSETS_FILENAME, std::fstream::out);

    if (!file.is_open())
    {
        printf("Save: error opening %s...\n", OFFSETS_FILENAME.c_str());
        return;
    }

    file << strBuff.GetString();

    file.close();
}

void FileManager::FormatPressureMatOffset(Writer<StringBuffer> &writer, pressure_mat_offset_t offset, string objectName)
{
    writer.Key(objectName.c_str());
    writer.StartObject();
    writer.Key("analogOffset");
    writer.StartArray();
    for (unsigned i = 0; i < PRESSURE_SENSOR_COUNT; i++)
    {
        writer.Int(offset.analogOffset[i]);
    }
    writer.EndArray();
    writer.Key("totalSensorMean");
    writer.Int(offset.totalSensorMean);
    writer.Key("detectionThreshold");
    writer.Double(offset.detectionThreshold);
    writer.EndObject();
}

void FileManager::FormatImuOffset(Writer<StringBuffer> &writer, imu_offset_t offset, string objectName)
{
    writer.Key(objectName.c_str());
    writer.StartObject();
    writer.Key("accelerometerOffsets");
    writer.StartArray();
    for (unsigned i = 0; i < NUMBER_OF_AXIS; i++)
    {
        writer.Int(offset.accelerometerOffsets[i]);
    }
    writer.EndArray();
    writer.Key("gyroscopeOffsets");
    writer.StartArray();
    for (unsigned i = 0; i < NUMBER_OF_AXIS; i++)
    {
        writer.Int(offset.gyroscopeOffsets[i]);
    }
    writer.EndArray();
    writer.EndObject();
}

imu_offset_t FileManager::ParseIMUOffset(Document &document, string objectName)
{
    imu_offset_t ret;

    if (document[objectName.c_str()].IsObject())
    {
        Value &object = document[objectName.c_str()];
        Value &jsonArray = object["accelerometerOffsets"];
        if (jsonArray.IsArray())
        {
            for (size_t i = 0; i < jsonArray.Size(); i++)
            {
                // printf("accelerometerOffsets[%d] = %d\n", i, a[i].GetInt());
                ret.accelerometerOffsets[i] = jsonArray[i].GetInt();
            }
        }
        jsonArray = object["gyroscopeOffsets"];
        if (jsonArray.IsArray())
        {
            for (size_t i = 0; i < jsonArray.Size(); i++)
            {
                // printf("gyroscopeOffsets[%d] = %d\n", i, a[i].GetInt());
                ret.gyroscopeOffsets[i] = jsonArray[i].GetInt();
            }
        }
    }

    return ret;
}

pressure_mat_offset_t FileManager::ParsePressureMatOffset(Document &document)
{
    pressure_mat_offset_t ret;

    if (document["pressure_mat_offset"].IsObject())
    {
        Value &object = document["pressure_mat_offset"];

        Value &jsonArray = object["analogOffset"];
        if (jsonArray.IsArray())
        {
            for (size_t i = 0; i < jsonArray.Size(); i++)
            {
                // printf("analogOffset[%d] = %d\n", i, a[i].GetInt());
                ret.analogOffset[i] = jsonArray[i].GetInt();
            }
        }

        // printf("%i \n", object["totalSensorMean"].GetInt());
        // printf("%f \n", object["detectionThreshold"].GetFloat());

        ret.totalSensorMean = object["totalSensorMean"].GetInt();
        ret.detectionThreshold = object["detectionThreshold"].GetFloat();
    }

    return ret;
}

imu_offset_t FileManager::GetMobileImuOffsets()
{
    imu_offset_t ret;

    if (_mobileImuOffset.gyroscopeOffsets[AXIS::x] != 0 && _mobileImuOffset.gyroscopeOffsets[AXIS::y] != 0 && _mobileImuOffset.gyroscopeOffsets[AXIS::z] != 0)
    {
        for (int i = 0; i < NUMBER_OF_AXIS; i++)
        {
            ret.gyroscopeOffsets[i] = _mobileImuOffset.gyroscopeOffsets[i];
        }
    }

    if (_mobileImuOffset.accelerometerOffsets[AXIS::x] != 0 && _mobileImuOffset.accelerometerOffsets[AXIS::y] != 0 && _mobileImuOffset.accelerometerOffsets[AXIS::z] != 0)
    {
        for (int i = 0; i < NUMBER_OF_AXIS; i++)
        {
            ret.accelerometerOffsets[i] = _mobileImuOffset.accelerometerOffsets[i];
        }
    }

    return ret;
}

imu_offset_t FileManager::GetFixedImuOffsets()
{
    imu_offset_t ret;

    if (_fixedImuOffset.gyroscopeOffsets[AXIS::x] != 0 && _fixedImuOffset.gyroscopeOffsets[AXIS::y] != 0 && _fixedImuOffset.gyroscopeOffsets[AXIS::z] != 0)
    {
        for (int i = 0; i < NUMBER_OF_AXIS; i++)
        {
            ret.gyroscopeOffsets[i] = _fixedImuOffset.gyroscopeOffsets[i];
        }
    }

    if (_fixedImuOffset.accelerometerOffsets[AXIS::x] != 0 && _fixedImuOffset.accelerometerOffsets[AXIS::y] != 0 && _fixedImuOffset.accelerometerOffsets[AXIS::z] != 0)
    {
        for (int i = 0; i < NUMBER_OF_AXIS; i++)
        {
            ret.accelerometerOffsets[i] = _fixedImuOffset.accelerometerOffsets[i];
        }
    }

    return ret;
}
