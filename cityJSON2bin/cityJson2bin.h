#pragma once

///////////////////////////////////////////////////////////////////////////////////////////
//
// convert cityJSON to RDF bin format
//


#ifndef CITYJSON2BIN_EXTERN
    #ifdef _MSC_VER
        #ifdef CITYJSON2BIN_BUILD
            #define CITYJSON2BIN_EXPORT __declspec(dllexport)
        #else
            #define CITYJSON2BIN_EXPORT __declspec(dllimport)
        #endif // CITYJSON2BIN_BUILD
    #else
        #define CITYJSON2BIN_EXPORT /**/
    #endif
#endif


/// <summary>
/// 
/// </summary>
typedef const char* cityJson2bin_error;


/// <summary>
/// 
/// </summary>
/// <param name="filePathCityJson"></param>
/// <param name="filePathBin"></param>
/// <returns></returns>
extern CITYJSON2BIN_EXPORT cityJson2bin_error cityJson2bin_Convert(
    const char* filePathCityJson,
    const char* filePathBin    
);

