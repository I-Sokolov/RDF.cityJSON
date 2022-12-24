
#include "pch.h"
#include "CommonDefs.h"
#include "CityModel.h"
#include "cityJson2bin.h"

//-----------------------------------------------------------------------------------------------
//
extern CITYJSON2BIN_EXPORT cityJson2bin_error cityJson2bin_Convert(
    const char* filePathCityJson,
    const char* filePathBin
)
{
    cityJson2bin_error error;

    try {
        CityModel city;
        city.Convert(filePathCityJson, filePathBin);
    }
    catch (cityJson2bin_error expt) {
        error = expt;
    }

    return error;
}

//---------------------------------------------------------------------------------
//

extern void JsonAssertionError (const char* assertion, const char* file, int line)
{
    char msg[512];
    snprintf(msg, 511, "JSON assertion '%s' failed at file %s line %d\n", assertion, file, line);
    THROW_ERROR(msg);
}

//---------------------------------------------------------------------------------
//

extern void THROW_ERROR(const char* error_code) 
{ 
    throw cityJson2bin_error(error_code); 
}

//---------------------------------------------------------------------------------
//

extern void LOG_CNV(const char* catergory, const char* msg)
{
    char m[512];
    snprintf(m, 511, "%s: %s", catergory, msg);
    
    static std::set<std::string> shown;

    if (shown.insert(m).second) {
        printf("%s\n", m);
    }
}
