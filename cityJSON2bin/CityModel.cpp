
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
    const char*         type        = nullptr;
    const char*         version     = nullptr;
    rapidjson::Value    transform;
    rapidjson::Value    cityObjects;

    for (auto& member : m_cityDOM.GetObject()) {
        auto memberName = member.name.GetString();
        if (!strcmp(memberName, MEMBER_TYPE)) {
            type = member.value.GetString();
        }
        else if (!strcmp(memberName, MEMBER_VERSION)) {
            version = member.value.GetString();
        }
        else if (!strcmp(memberName, MEMBER_VERTICIES)) {
            m_geometry.SetCityVerticies (member.value);
        }
        else if (!strcmp(memberName, MEMBER_APPEARANCE)) {
            m_appearance.SetCityAppearance(member.value);
        }
        else if (!strcmp(memberName, MEMBER_TRANSFORM)) {
            transform = member.value;
        }
        else if (!strcmp(memberName, MEMBER_GEOM_TEMPLATES)) {
            m_geometry.SetGeometryTemplates(member.value);
        }
        else if (!strcmp(memberName, MEMBER_CITYOBJECTS)) {
            cityObjects = member.value;
        }
        else {
            LOG_CNV("Unsupported cityJSON member", memberName);
        }
    }

    if (!type || strcmp(type, TYPE_CityJSON))
        THROW_ERROR("Expcected type CityJSON");
    if (!version || fabs(atof(version) - VERSION_1_1) > DBL_MIN)
        THROW_ERROR("Unsupported version");

    std::vector<OwlInstance> owlObjects;
    int iObject = 0;
    for (auto& o : cityObjects.GetObject()) {
        iObject++;
        //if (iObject != 6)
        //    continue;

        auto id = o.name.GetString();
        auto& cityObject = o.value;

        auto instance = ConvertCityObject(id, cityObject);
        if (instance) {
            owlObjects.push_back(instance);
        }
    }

    const char* clsname[] = { type , OWL_Collection, NULL};
    auto cls = GetOrCreateClass(clsname);
    GEOM::Collection city = CreateInstance(cls, type);

    //TODO SetCityJSONTransform(jtransform);

    city.set_objects(owlObjects.data(), owlObjects.size());
}


//-----------------------------------------------------------------------------------------------
//
OwlInstance CityModel::ConvertCityObject(const char* id, rapidjson::Value& jobject)
{
    rapidjson::Value jtype;
    rapidjson::Value jgeometry;
    
    for (auto& member : jobject.GetObject()) {
        auto memberName = member.name.GetString();
        if (!strcmp(memberName, MEMBER_TYPE)) {
            jtype = member.value;
        }
        else if (!strcmp(memberName, MEMBER_GEOMETRY)) {
            jgeometry = member.value;
        }
        else {
            LOG_CNV("Unsupported city object member", memberName);
        }
    }

    auto type = jtype.GetString();
    //if (!_stricmp(type, "TINRelief"))
    //    return 0;

    std::vector<GEOM::GeometricItem> items;
    if (jgeometry.IsArray()) {
        m_geometry.Convert(jgeometry, items);
    }
    else {
        LOG_CNV("City object has no geometry", type);
    }
    if (items.empty()) {
        return 0;
    }

    std::string owlType(OWL_CityJsonPrefix);
    owlType.append(type);

    const char* clsname[] = { owlType.c_str() , OWL_Collection, NULL };
    auto cls = GetOrCreateClass(clsname);
    GEOM::Collection instance = CreateInstance(cls, id);
    instance.set_objects(items.data(), items.size());

    return instance;
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


//-----------------------------------------------------------------------------------------------
//
RdfProperty CityModel::GetOrCreateProperty(OwlClass cls, const char* propName, RdfPropertyType propType, int64_t minCard, int64_t maxCard, int attempt)
{
    //std::string fullPropName = GetNameOfClass(cls);
    //fullPropName.append(".");
    //fullPropName.append(propName);
    std::string fullPropName (propName);
    if (attempt > 0) {
        char attmpt[64];
        sprintf(attmpt, "_cityJson%d", attempt);
        fullPropName.append(attmpt);
    }

    auto prop = GetPropertyByName(m_owlModel, fullPropName.c_str());
    if (prop) {
        auto ptype = GetPropertyType(prop);
        int64_t minC = 0;
        int64_t maxC = 0;
        GetClassPropertyCardinalityRestriction(cls, prop, &minC, &maxC);
        if (ptype != propType || minC != minCard || maxC != maxCard) {
            LOG_CNV("Porperty exists but traits mismatches", fullPropName.c_str());
            prop = GetOrCreateProperty(cls, propName, propType, minCard, maxCard, attempt + 1);
        }
    }
    else {
        prop = CreateProperty(m_owlModel, propType, fullPropName.c_str());
        SetClassPropertyCardinalityRestriction(cls, prop, minCard, maxCard);
    }

    return prop;
}

//-----------------------------------------------------------------------------------------------
//
void CityModel::CreateAttribute(OwlInstance instance, const char* name, rapidjson::Value& value)
{
    auto cls = GetInstanceClass(instance);
    auto ktype = value.GetType();

    switch (ktype) {
        case rapidjson::kStringType:
        {
            auto val = value.GetString();
            auto prop = GetOrCreateProperty(cls, name, DATATYPEPROPERTY_TYPE_CHAR);
            SetDatatypeProperty(instance, prop, val);
            break;
        }

        case rapidjson::kNumberType:
        {
            auto val = value.GetDouble();
            auto prop = GetOrCreateProperty(cls, name, DATATYPEPROPERTY_TYPE_DOUBLE);
            SetDatatypeProperty(instance, prop, val);
            break;
        }

        default:
            LOG_CNV("Unsupported attribte type", name);
    }
}
