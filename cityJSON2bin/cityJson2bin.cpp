
#include "pch.h"
#include "cityJson2bin.h"

//-----------------------------------------------------------------------------------------------
//
static bool ReadFile(rapidjson::Document& cityDOM, const char* fileJson)
{
    const char* ReadMode = "rb";
#ifndef WINDOWS
    ReadMode = "r";
#endif // !WINDOWS

    FILE* fpInput = fopen(fileJson, ReadMode);
    if (!fpInput) {
        return false;
    }

    static char readBuff[65536];
    rapidjson::FileReadStream rstream(fpInput, readBuff, sizeof(readBuff));

    cityDOM.ParseStream(rstream);

    fclose(fpInput);

    return cityDOM.IsObject();
}

//-----------------------------------------------------------------------------------------------
//
extern CITYJSON2BIN_EXPORT enum_cityJson2bin_result cityJson2bin_Convert(
    const char* filePathCityJson,
    const char* filePathBin
)
{
    rapidjson::Document cityDOM;
    if (!ReadFile(cityDOM, filePathCityJson)) {
        return enum_cityJson2bin_result::FailRead;
    }


    if (cityDOM.HasMember("type")) {
        printf("Type %s\n", cityDOM["type"].GetString());
    }
    else {
        printf("Error\n");
    }

    return enum_cityJson2bin_result::OK;
}

