// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#undef max
#undef GetObject

#include <stdio.h>
#include <assert.h>
#include <stdarg.h>

#include <list>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <filesystem>

#define RAPIDJSON_ASSERT(x) {if(!(x)) {JsonAssertionError(#x,__FILE__,__LINE__);}}
extern void JsonAssertionError(const char* assertion, const char* file, int line);

#include <rapidjson.h>
#include <document.h>
#include <filereadstream.h>

#include <geom.h>

#define CITYJSONRDF_BUILD_DLL_INTERNAL

#endif //PCH_H
