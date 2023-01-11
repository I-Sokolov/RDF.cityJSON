
#include "pch.h"
#include "CommonDefs.h"
#include "CityModel.h"
#include "CityJsonRDF.h"

//-----------------------------------------------------------------------------------------------
//
static CityModel* s_pModel = NULL;

//-----------------------------------------------------------------------------------------------
//
extern CITYJSONRDF_EXPORT OwlModel CityJsonRDF::Open(
    const char*                 filePathCityJson,
    CityJsonRDF::IProgress*     pProgress,
    CityJsonRDF::ILog*          pLog
)
{
    if (s_pModel) {
        if (pLog) {
            pLog->Message(ILog::Level::Error, "Another model is opening now", "Another model is opening now", "<not initialized");
        }
        return NULL;
    }

    s_pModel = new CityModel (pProgress, pLog);

    OwlModel model = NULL;
    
    try {
        model = s_pModel->Open(filePathCityJson);
    }
    catch (CityModel::Exception) {
        s_pModel->LogMessage(ILog::Level::Error, "Failed to read model");
    }

    delete s_pModel;
    s_pModel = NULL;

    return model;
}

//---------------------------------------------------------------------------------
//

void JsonAssertionError(const char* assertion, const char* /*file*/, int /*line*/)
{
    assert(s_pModel);
    if (s_pModel) {
        s_pModel->ThrowError("error in JSON data: '%s'", assertion);
    }
}


