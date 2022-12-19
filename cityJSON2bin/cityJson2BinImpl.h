// IMPLEMENTATION DECLARATIONS
#pragma once

#define ERROR(error_code) {throw cityJson2bin_error(error_code);}

class CCityJson2Bin
{
public:
    CCityJson2Bin(){}

public:
    void Convert(const char* cityFilePath, const char* rdfFilePath);

private:
    void ReadCityFile(const char* cityFilePath);

    void ConvertCityJSONObject();
    void GetCityJSONTransform(rapidjson::Value& jtransform);
    void GetCityJSONVerticies(rapidjson::Value& jverticies);

    void ConvertCityObject(const char* id, rapidjson::Value& jobject);


private:
    rapidjson::Document     m_cityDOM;
    rapidjson::Value        m_jverticies;
};
