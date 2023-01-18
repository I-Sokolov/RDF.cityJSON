// IMPLEMENTATION DECLARATIONS
#pragma once

#include "CityJsonRDF.h"
using namespace CityJsonRDF;
#include "CommonDefs.h"
#include "Geometry.h"
#include "Appearance.h"
#include "Settings.h"

class CityModel
{
public:
    struct Exception {};

public:
    CityModel(CityJsonRDF::IProgress* pProgress, CityJsonRDF::ILog* pLog);
    ~CityModel();

public:
    OwlModel Open(const char* cityFilePath);

public:
    Settings& GetSettings() { return m_settings; }

    OwlModel RdfModel() { return m_owlModel; }    
    OwlClass GetOrCreateClass(const char* name, bool addPrefix, const char* parent1 = nullptr, const char* parent2 = nullptr);
    void AddClassParent(OwlClass cls, const char* parentName);
    RdfProperty GetOrCreateProperty(OwlClass cls, const char* propName, const char* prefix, RdfPropertyType propType, const char* refCls = nullptr, int64_t minCard = 0, int64_t maxCard = 1, int attempt = 0);
    void CreateAttribute(OwlInstance instance, const char* name, const char* prefix, rapidjson::Value& value);

    Appearance& GetAppearance() { return m_appearance; }
    Geometry& GetGeometry() { return m_geometry; }

    ConverterState& State() { return m_converterState; }

public:
    void ThrowError(const char* fmt, ...);
    void LogMessage(ILog::Level level, const char* fmt, ...);

private:
    struct CityObject {
        OwlInstance  owlObject = NULL;
        ListOfString parents;
        ListOfString children;
    };

    typedef std::map<std::string, CityObject> CityObjects;

private:
    void ReadCityFile(const char* cityFilePath);

    void InitOntology();
    void AddNestedObjects(OwlInstance instance, const char* propName, OwlInstances& value);
    OwlInstance ConvertAttributeObject(const char* name, rapidjson::Value& value);

    void ConvertCityJSONObject();
    void ConvertCityObject(CityObject& object, rapidjson::Value& id, rapidjson::Value& jobject);
    void SetupChildren(CityObjects& objects, OwlInstances& topLevel);

private:
    CityJsonRDF::IProgress* m_pProgress;
    CityJsonRDF::ILog*      m_pLog;

    OwlModel                m_owlModel;
    rapidjson::Document     m_cityDOM;
    ConverterState          m_converterState;

    Geometry                m_geometry;
    Appearance              m_appearance;
    Settings                m_settings;
};
