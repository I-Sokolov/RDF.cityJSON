
#include "pch.h"
#include "CommonDefs.h"
#include "Geometry.h"
#include "cityJson2bin.h"
#include "CityModel.h"

//---------------------------------------------------------------------------------
//
CityModel::CityModel()
    : m_geometry(*this)
    , m_appearance(*this)
{
    m_owlModel = CreateModel();
}

//---------------------------------------------------------------------------------
//
CityModel::~CityModel()
{
    CloseModel(m_owlModel);
}


//---------------------------------------------------------------------------------
//
void CityModel::Convert(const char* cityFilePath, const char* rdfFilePath)
{
    ReadCityFile(cityFilePath);

    ConvertCityJSONObject();

    SaveBinFile(rdfFilePath);    
}


//-----------------------------------------------------------------------------------------------
//
void CityModel::SaveBinFile(const char* rdfFilePath)
{
    auto fp = fopen(rdfFilePath, "w");
    if (!fp)
        THROW_ERROR("Filed to open output file");
    fclose(fp);

    auto res = SaveModel(m_owlModel, rdfFilePath);
    if (res)
        THROW_ERROR("Failed to write output file");
}

//-----------------------------------------------------------------------------------------------
//
void CityModel::ReadCityFile(const char* cityFilePath)
{
    const char* ReadMode = "rb";
#ifndef WINDOWS
    ReadMode = "r";
#endif // !WINDOWS

    FILE* fpInput = fopen(cityFilePath, ReadMode);
    if (!fpInput)
        THROW_ERROR("Failed to open input file");

    static char readBuff[65536];
    rapidjson::FileReadStream rstream(fpInput, readBuff, sizeof(readBuff));

    m_cityDOM.ParseStream(rstream);

    fclose(fpInput);
}

//-----------------------------------------------------------------------------------------------
//
void CityModel::ConvertCityJSONObject()
{
    auto type = m_cityDOM[MEMBER_TYPE].GetString();
    if (strcmp(type, TYPE_CityJSON))
        THROW_ERROR("Expcected type CityJSON");
    
    const char* clsname[] = { type , OWL_Collection, NULL};
    auto cls = GetOrCreateClass(clsname);
    GEOM::Collection city = CreateInstance(cls, type);

    auto sversion = m_cityDOM[MEMBER_VERSION].GetString();
    auto version = atof(sversion);
    if (fabs(version - VERSION_1_1) > DBL_MIN)
        THROW_ERROR("Unsupported version");

    //auto& jtransform = m_cityDOM[MEMBER_TRANSFORM];
    //TODO SetCityJSONTransform(jtransform);

    auto& jverticies = m_cityDOM[MEMBER_VERTICIES];
    m_jcityVerticies = jverticies;

    auto itAppearance = m_cityDOM.FindMember(MEMBER_APPEARANCE);
    if (itAppearance != m_cityDOM.MemberEnd()) {
        m_appearance.SetCityAppearance(itAppearance->value);
    }

    std::vector<OwlInstance> objects;
    for (auto& o : m_cityDOM[MEMBER_CITYOBJECTS].GetObject()) {
        auto id = o.name.GetString();
        auto& cityObject = o.value;

        auto instance = ConvertCityObject(id, cityObject);
        if (instance) {
            objects.push_back(instance);
        }
    }

    city.set_objects(objects.data(), objects.size());
}


//-----------------------------------------------------------------------------------------------
//
OwlInstance CityModel::ConvertCityObject(const char* id, rapidjson::Value& jobject)
{
    try {
        auto& jtype = jobject[MEMBER_TYPE];
        auto type = jtype.GetString();

        std::string owlType(OWL_CityJsonPrefix);
        owlType.append(type);

        const char* clsname[] = { owlType.c_str() , OWL_Collection, NULL };
        auto cls = GetOrCreateClass(clsname);
        GEOM::Collection instance = CreateInstance(cls, id);

        auto& jgeometry = jobject[MEMBER_GEOMETRY];
        std::vector<GEOM::GeometricItem> items;
        m_geometry.Convert(jgeometry, items);
        instance.set_objects(items.data(), items.size());

        return instance;
    }
    catch (cityJson2bin_error expt) {
        LOG_CNV("Failed to convert object", expt.c_str());        
    }
    return 0;
}

//-----------------------------------------------------------------------------------------------
//
OwlClass CityModel::GetOrCreateClass(const char* names[])
{
    if (!names || !names[0]) {
        return NULL;
    }

    auto cls = GetClassByName(m_owlModel, names[0]);
    
    if (!cls) {
        cls = CreateClass(m_owlModel, names[0]);

        auto parent = GetOrCreateClass(names + 1);
        if (parent) {
            SetClassParent(cls, parent, 1);
        }
    }

    return cls;
}
