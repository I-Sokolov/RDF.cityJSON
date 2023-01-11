#pragma once

///////////////////////////////////////////////////////////////////////////////////////////
//
// convert cityJSON to RDF bin format
//


#ifndef CITYJSON2BIN_EXTERN
    #ifdef CITYJSON2BIN_BUILD_DLL_INTERNAL
        #define CITYJSON2BIN_EXPORT __declspec(dllexport)
    #elif CITYJSON2BIN_USE_DLL
        #define CITYJSON2BIN_EXPORT __declspec(dllimport)
    #else
        #define CITYJSON2BIN_EXPORT /**/
    #endif
#endif

namespace cityJson2bin
{
    /// <summary>
    /// 
    /// </summary>
    struct IProgress
    {
        virtual void Start(int range) = NULL;
        virtual void Step() = NULL;
        virtual void Finish() = NULL;
    };

    /// <summary>
    /// 
    /// </summary>
    struct ILog
    {
        enum class Level { Error, Warning, Info };
        virtual void Message(Level level, const char* category, const char* msg, const char* converterState) = NULL;
    };

    /// <summary>
    /// 
    /// </summary>
    extern CITYJSON2BIN_EXPORT OwlModel Open(
        const char* filePathCityJson,
        IProgress* pProgress = NULL,
        ILog* pLog = NULL
    );

}