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

    const rapidjson::Value& GetVertex(int ind) { return m_jcityVerticies[ind]; }
    Appearance& GetAppearance() { return m_appearance; }

private:
    void ReadCityFile(const char* cityFilePath);
    void SaveBinFile(const char* rdfFilePath);

    void ConvertCityJSONObject();

    OwlInstance ConvertCityObject(const char* id, rapidjson::Value& jobject);

private:
    OwlModel                m_owlModel;
    rapidjson::Document     m_cityDOM;

    rapidjson::Value        m_jcityVerticies;
    Geometry                m_geometry;
    Appearance              m_appearance;
};
