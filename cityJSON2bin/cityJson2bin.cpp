
#include "pch.h"
#include "CommonDefs.h"
#include "CityJson.h"
#include "cityJson2bin.h"

//-----------------------------------------------------------------------------------------------
//
extern CITYJSON2BIN_EXPORT cityJson2bin_error cityJson2bin_Convert(
    const char* filePathCityJson,
    const char* filePathBin
)
{
    cityJson2bin_error error = 0;

    try {
        CityJson city;
        city.Convert(filePathCityJson, filePathBin);
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
    THROW_ERROR("Something unexpected in JSON");
}

//---------------------------------------------------------------------------------
//

extern void THROW_ERROR(const char* error_code) 
{ 
    throw cityJson2bin_error(error_code); 
}
