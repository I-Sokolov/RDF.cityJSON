// IMPLEMENTATION DECLARATIONS
#pragma once

#include "Geometry.h"

class CityModel
{
public:
    CityModel();
    ~CityModel();

public:
    void Convert(const char* cityFilePath, const char* rdfFilePath);

public:
    OwlModel GetModel() { return m_owlModel; }
    OwlClass GetOrCreateClass(const char* className, const char* parentName);

private:
    void ReadCityFile(const char* cityFilePath);
    void SaveBinFile(const char* rdfFilePath);

    void ConvertCityJSONObject();

    void ConvertCityObject(const char* id, rapidjson::Value& jobject);

private:
    OwlModel                m_owlModel;
    rapidjson::Document     m_cityDOM;

    Geometry                m_geometry;
};
