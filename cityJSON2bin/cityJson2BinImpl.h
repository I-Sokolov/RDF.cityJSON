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

private:
    rapidjson::Document m_cityDOM;
};
