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
    OwlClass GetOrCreateClass(const char* names[] /*from this to parents*/, bool addPrefix);
    RdfProperty GetOrCreateProperty(OwlClass cls, const char* propName, const char* prefix, RdfPropertyType propType, const char* refCls = nullptr, int64_t minCard = 0, int64_t maxCard = 1, int attempt = 0);
    void CreateAttribute(OwlInstance instance, const char* name, rapidjson::Value& value);

    Appearance& GetAppearance() { return m_appearance; }
    Geometry& GetGeometry() { return m_geometry; }

private:
    struct CityObject {
        OwlInstance  owlObject = NULL;
        ListOfString parents;
        ListOfString children;
    };

    typedef std::map<std::string, CityObject> CityObjects;

private:
    void ReadCityFile(const char* cityFilePath);
    void SaveBinFile(const char* rdfFilePath);

    void CreateCommonClasses();
    void SetProperty(OwlInstance instance, const char* propName, OwlInstances& value);
    OwlInstance ConvertAttributeObject(const char* name, rapidjson::Value& value);

    void ConvertCityJSONObject();
    void ConvertCityObject(CityObject& object, rapidjson::Value& id, rapidjson::Value& jobject);
    void SetupChildren(CityObjects& objects, OwlInstances& topLevel);


private:
    OwlModel                m_owlModel;
    rapidjson::Document     m_cityDOM;

    Geometry                m_geometry;
    Appearance              m_appearance;
};
