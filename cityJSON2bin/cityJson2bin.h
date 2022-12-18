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
enum class enum_cityJson2bin_result : unsigned char
{
    OK = 0,
    FailRead
};


/// <summary>
/// 
/// </summary>
/// <param name="filePathCityJson"></param>
/// <param name="filePathBin"></param>
/// <returns></returns>
extern CITYJSON2BIN_EXPORT enum_cityJson2bin_result cityJson2bin_Convert(
    const char* filePathCityJson,
    const char* filePathBin    
);

