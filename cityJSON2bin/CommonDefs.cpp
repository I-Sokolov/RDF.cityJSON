
#include "pch.h"
#include "CommonDefs.h"
#include "cityJson2bin.h"

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
    ERROR("Something unexpected in JSON");
}

