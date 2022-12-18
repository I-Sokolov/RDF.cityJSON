
#include "pch.h"
#include "cityJson2binAPI.h"
#include "cityJson2binImpl.h"
#include "Keywords.h"

//-----------------------------------------------------------------------------------------------
//
extern CITYJSON2BIN_EXPORT cityJson2bin_error cityJson2bin_Convert(
    const char* filePathCityJson,
    const char* filePathBin
)
{
    cityJson2bin_error error = 0;

    try {
        CCityJson2Bin conv;
        conv.Convert(filePathCityJson, filePathBin);
    }
    catch (cityJson2bin_error expt) {
        error = expt;
    }

    return error;
}

//---------------------------------------------------------------------------------
//

extern void JsonAssertionError
#ifdef _DEBUG
(const char* assertion, const char* file, int line)
#else
(const char*, const char*, int)
#endif
{
#ifdef _DEBUG
    printf("JSON assertion '%s' failed at file %s line %d\n", assertion, file, line);
#endif
    ERROR("Unexpected JSON");
}

//---------------------------------------------------------------------------------
//
void CCityJson2Bin::Convert (const char* cityFilePath, const char* /*rdfFilePath*/)
{
    ReadCityFile(cityFilePath);

    ConvertCityJSONObject();
}

//-----------------------------------------------------------------------------------------------
//
void CCityJson2Bin::ReadCityFile(const char* cityFilePath)
{
    const char* ReadMode = "rb";
#ifndef WINDOWS
    ReadMode = "r";
#endif // !WINDOWS

    FILE* fpInput = fopen(cityFilePath, ReadMode);
    if (!fpInput)
        ERROR("Failed to open file");

    static char readBuff[65536];
    rapidjson::FileReadStream rstream(fpInput, readBuff, sizeof(readBuff));

    m_cityDOM.ParseStream(rstream);

    fclose(fpInput);
}

//-----------------------------------------------------------------------------------------------
//
void CCityJson2Bin::ConvertCityJSONObject()
{
    auto type = m_cityDOM[MEMBER_TYPE].GetString();
    if (strcmp(type, TYPE_CityJSON))
        ERROR("Expcected type CityJSON");

    auto strversion = m_cityDOM[MEMBER_VERSION].GetString();
    auto version = atof(strversion);
    if (fabs(version - VERSION_1_1) > DBL_MIN)
        ERROR("Unsupported version");

    auto& transform = m_cityDOM[MEMBER_TRANSFORM];
    //GetCityJSONTransform();


}
