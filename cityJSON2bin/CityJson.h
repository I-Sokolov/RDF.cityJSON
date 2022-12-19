// IMPLEMENTATION DECLARATIONS
#pragma once

class CityJson
{
public:
    CityJson();
    ~CityJson();

public:
    void Convert(const char* cityFilePath, const char* rdfFilePath);

private:
    void ReadCityFile(const char* cityFilePath);
    void SaveBinFile(const char* rdfFilePath);

    void ConvertCityJSONObject();
    void GetCityJSONTransform(rapidjson::Value& jtransform);
    void GetCityJSONVerticies(rapidjson::Value& jverticies);

    void ConvertCityObject(const char* id, rapidjson::Value& jobject);

private:
    OwlModel                m_owlDOM;
    rapidjson::Document     m_cityDOM;

    rapidjson::Value        m_jverticies;

};
