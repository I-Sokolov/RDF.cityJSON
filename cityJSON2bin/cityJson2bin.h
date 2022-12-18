#pragma once

///////////////////////////////////////////////////////////////////////////////////////////
//
// convert cityJSON to RDF bin format
//


#ifndef CITYJSON2BIN_EXTERN
#ifdef MSC_VER
#ifdef CITYJSON2BIN_BUILD
#define CITYJSON2BIN_EXPORT __declspec(dllexport)
#else
#define CITYJSON2BIN_EXPORT __declspec(dllimport)
#endif // CITYJSON2BIN_BUILD
#else // MSC_VER
#define CITYJSON2BIN_EXPORT /**/
#endif
#endif


extern CITYJSON2BIN_EXPORT void cityJson2bin_Convert(
    const char* filePathCityJson,
    const char* filePathBin
);

