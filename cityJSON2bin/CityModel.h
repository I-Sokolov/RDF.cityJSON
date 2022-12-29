// IMPLEMENTATION DECLARATIONS
#pragma once

#include "Geometry.h"
#include "Appearance.h"

class CityModel
{
public:
    CityModel();
    ~CityModel();

public:
    void Convert(const char* cityFilePath, const char* rdfFilePath);

public:
    OwlModel RdfModel() { return m_owlModel; }    
    OwlClass GetOrCreateClass(const char* names[] /*from this to parents*/);
    RdfProperty GetOrCreateProperty(OwlClass cls, const char* propName, RdfPropertyType propType, int64_t minCard = 0, int64_t maxCard = 1, int attempt = 0);
    void CreateAttribute(OwlInstance instance, const char* name, rapidjson::Value& value);

    Appearance& GetAppearance() { return m_appearance; }
    Geometry& GetGeometry() { return m_geometry; }

private:
    void ReadCityFile(const char* cityFilePath);
    void SaveBinFile(const char* rdfFilePath);

    void ConvertCityJSONObject();

    OwlInstance ConvertCityObject(const char* id, rapidjson::Value& jobject);

private:
    OwlModel                m_owlModel;
    rapidjson::Document     m_cityDOM;

    Geometry                m_geometry;
    Appearance              m_appearance;
};
