
#include "pch.h"
#include "CommonDefs.h"
#include "Geometry.h"
#include "CityJson.h"


//---------------------------------------------------------------------------------
//
void CityJson::Convert (const char* cityFilePath, const char* /*rdfFilePath*/)
{
    ReadCityFile(cityFilePath);

    ConvertCityJSONObject();
}

//-----------------------------------------------------------------------------------------------
//
void CityJson::ReadCityFile(const char* cityFilePath)
{
    const char* ReadMode = "rb";
#ifndef WINDOWS
    ReadMode = "r";
#endif // !WINDOWS

    FILE* fpInput = fopen(cityFilePath, ReadMode);
    if (!fpInput)
        THROW_ERROR("Failed to open file");

    static char readBuff[65536];
    rapidjson::FileReadStream rstream(fpInput, readBuff, sizeof(readBuff));

    m_cityDOM.ParseStream(rstream);

    fclose(fpInput);
}

//-----------------------------------------------------------------------------------------------
//
void CityJson::ConvertCityJSONObject()
{
    auto jtype = m_cityDOM[MEMBER_TYPE].GetString();
    if (strcmp(jtype, TYPE_CityJSON))
        THROW_ERROR("Expcected type CityJSON");

    auto sversion = m_cityDOM[MEMBER_VERSION].GetString();
    auto version = atof(sversion);
    if (fabs(version - VERSION_1_1) > DBL_MIN)
        THROW_ERROR("Unsupported version");

    auto& jtransform = m_cityDOM[MEMBER_TRANSFORM];
    GetCityJSONTransform(jtransform);

    auto& jverticies = m_cityDOM[MEMBER_VERTICIES];
    GetCityJSONVerticies(jverticies);

    for (auto& o : m_cityDOM[MEMBER_CITYOBJECTS].GetObject()) {
        auto id = o.name.GetString();
        auto& cityObject = o.value;
        ConvertCityObject(id, cityObject);
    }
}

//-----------------------------------------------------------------------------------------------
//
void CityJson::GetCityJSONVerticies(rapidjson::Value& jverticies)
{
    assert(jverticies.IsArray());
    m_jverticies = jverticies;
    assert(jverticies.IsNull());
}

//-----------------------------------------------------------------------------------------------
//
void CityJson::GetCityJSONTransform(rapidjson::Value& /*jtransform*/)
{
    //TODO
}

//-----------------------------------------------------------------------------------------------
//
void CityJson::ConvertCityObject(const char* id, rapidjson::Value& jobject)
{
    auto& jtype = jobject[MEMBER_TYPE];
    printf("%s is %s\n", id, jtype.GetString());

    auto& jgeometry = jobject[MEMBER_GEOMETRY];
    Geometry geom(*this);
    geom.Convert(jgeometry);
}
