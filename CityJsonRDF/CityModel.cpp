
#include "pch.h"
#include "CommonDefs.h"
#include "Geometry.h"
#include "CityJsonRDF.h"
using namespace CityJsonRDF;
#include "CityModel.h"

//---------------------------------------------------------------------------------
//
CityModel::CityModel(CityJsonRDF::IProgress* pProgress, CityJsonRDF::ILog* pLog)
    : m_geometry(*this)
    , m_appearance(*this)
    , m_pProgress (pProgress)
    , m_pLog (pLog)
{
    m_owlModel = CreateModel();
}

//---------------------------------------------------------------------------------
//
CityModel::~CityModel()
{
    if (m_owlModel) {
        CloseModel(m_owlModel);
        m_owlModel = NULL;
    }
}


//---------------------------------------------------------------------------------
//
OwlModel CityModel::Open(const char* cityFilePath)
{
    ReadCityFile(cityFilePath);

    CreateBaseClasses();

    ConvertCityJSONObject();

    auto ret = m_owlModel;
    m_owlModel = NULL;
    
    return ret;
}


//-----------------------------------------------------------------------------------------------
//
void CityModel::ThrowError(const char* fmt, ...)
{
    char msg[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(msg, 511, fmt, args);
    va_end(args);

    if (m_pLog) {
        m_pLog->Message(ILog::Level::Error, fmt, msg, m_converterState.ToString().c_str());
    }

    throw Exception();
}

//-----------------------------------------------------------------------------------------------
//
void CityModel::LogMessage(ILog::Level level, const char* fmt, ...)
{
    char msg[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(msg, 511, fmt, args);
    va_end(args);

    if (m_pLog) {
        m_pLog->Message(level, fmt, msg, m_converterState.ToString().c_str());
    }
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
        ThrowError("Failed to open file %s", cityFilePath);

    static char readBuff[65536];
    rapidjson::FileReadStream rstream(fpInput, readBuff, sizeof(readBuff));

    m_cityDOM.ParseStream(rstream);

    fclose(fpInput);
}

//-----------------------------------------------------------------------------------------------
//
void CityModel::CreateBaseClasses()
{
    const char* clsnameGenericObject[] = { OWL_ClsCityJSONGenericObject, NULL};
    auto clsGenericObject = GetOrCreateClass(clsnameGenericObject, false);
    
    GetOrCreateProperty(clsGenericObject, OWL_PropRepresentation, NULL, OBJECTPROPERTY_TYPE, OWL_GeometricItem, 0, -1);
    GetOrCreateProperty(clsGenericObject, OWL_PropChildren, NULL, OBJECTPROPERTY_TYPE, OWL_ClsCityJSONGenericObject, 0, -1);

    const char* clsnameGeometricItem[] = { OWL_ClsGeomItem, OWL_Collection, NULL };
    /*auto clsGeometricItem =*/ GetOrCreateClass(clsnameGeometricItem, false);

    //GetOrCreateProperty(clsGeometricItem, OWL_PropLOD, NULL, DATATYPEPROPERTY_TYPE_CHAR);
}

//-----------------------------------------------------------------------------------------------
//
void CityModel::ConvertCityJSONObject()
{
    const char*         type        = nullptr;
    const char*         version     = nullptr;
    rapidjson::Value    transform;
    rapidjson::Value    cityObjects;
    rapidjson::Value    metadata;
    rapidjson::Value    extensions;

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
            State().PushMember(memberName);
            m_appearance.SetCityAppearance(member.value);
            State().Pop();
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
        else if (!strcmp(memberName, MEMBER_METADATA)) {
            metadata = member.value;
        }
        else if (!strcmp(memberName, MEMBER_EXTENSIONS)) {
            extensions = member.value;
        }
        else {
            LogMessage(ILog::Level::Info, "Unsupported CityJSON Object member: %s", memberName);
        }
    }

    if (!type || strcmp(type, TYPE_CityJSON))
        LogMessage(ILog::Level::Error, "Unexpected CityJSON Object type: '%s'", type?type:"(NULL)");

    if (!version || fabs(atof(version) - VERSION_1_1) > DBL_MIN)
        LogMessage(ILog::Level::Error, "Unsupported version: '%s'", version ? version : "(NULL)");

    //
    //
    if (m_pProgress) {
        auto memCount = cityObjects.GetObject().MemberCount();
        m_pProgress->Start((int)memCount);
    }

    m_converterState.PushMember(MEMBER_CITYOBJECTS);

    CityObjects objects;
    for (auto& o : cityObjects.GetObject()) {

        auto& id = o.name;
        m_converterState.PushMember(id.GetString());

        auto& cityObject = o.value;

        try {
            CityObject& object = objects[id.GetString()];
            ConvertCityObject(object, id, cityObject);
        }
        catch (Exception) {
            LogMessage(ILog::Level::Error, "Failed to convert city object");
        }

        m_converterState.Pop();

        if (m_pProgress) {
            m_pProgress->Step();
        }
    }

    m_converterState.Pop();

    if (m_pProgress) {
        m_pProgress->Finish();
    }

    //
    //
    OwlInstances owlObjects;
    SetupChildren(objects, owlObjects);

    //
    //
    const char* clsname[] = { type , OWL_ClsCityJSONGenericObject, NULL};
    auto cls = GetOrCreateClass(clsname, true);
    OwlInstance city = CreateInstance(cls, type);

    //
    //
    AddNestedObjects(city, OWL_PropChildren, owlObjects);

    CreateAttribute(city, MEMBER_TRANSFORM, OWL_PropCityJsonPrefix, transform);
    CreateAttribute(city, MEMBER_METADATA, OWL_PropCityJsonPrefix, metadata);
}

//-----------------------------------------------------------------------------------------------
//
void CityModel::AddNestedObjects(OwlInstance instance, const char* propName, OwlInstances& value)
{
    auto prop = GetPropertyByName(m_owlModel, propName);
    if (!prop) {
        assert(prop);
        return; //>>>>>>
    }

    int64_t* rold = NULL;
    int64_t nold = 0;
    GetObjectProperty(instance, prop, &rold, &nold);
    for (int64_t i = 0; i < nold; i++) {
        value.push_back(rold[i]);
    }

    SetObjectTypeProperty(instance, prop, value.data(), value.size());
}

//-----------------------------------------------------------------------------------------------
//
#ifdef DELETE_EMPTY_OBJECTS
static bool HasRepresentation(OwlInstance instance)
{
    auto model = GetModel(instance);
    auto prop = GetPropertyByName(model, OWL_PropRepresentation);
    if (!prop) {
        assert(prop);
        return false; //>>>>>>
    }

    int64_t* r = NULL;
    int64_t n = 0;
    GetObjectProperty(instance, prop, &r, &n);

    return n > 0;
}
#endif

//-----------------------------------------------------------------------------------------------
//
void CityModel::SetupChildren(CityObjects& objects, OwlInstances& topLevel)
{
    // complete parent-children
    //
    for (auto& object : objects) {
        for (auto& parentId : object.second.parents) {
            auto parent = objects[parentId];
            parent.children.insert(object.first);
        }

        for (auto& childId : object.second.children) {
            auto child = objects[childId];
            child.parents.insert(object.first);
        }
    }

    //
    //
    for (auto& object : objects) {
        if (object.second.owlObject) {

#ifdef DELETE_EMPTY_OBJECTS
            if (object.second.children.empty() && !HasRepresentation(object.second.owlObject)) {
                RemoveInstance(object.second.owlObject);
                object.second.owlObject = NULL;
                continue;
            }
#endif

            OwlInstances owlChildren;
            for (auto& childId : object.second.children) {
                auto child = objects[childId];
                if (child.owlObject) {
                    owlChildren.push_back(child.owlObject);
                }
            }

            if (!owlChildren.empty()) {
                AddNestedObjects(object.second.owlObject, OWL_PropChildren, owlChildren);
            }

            if (object.second.parents.empty()) {
                topLevel.push_back(object.second.owlObject);
            }
        }
    }
}


//-----------------------------------------------------------------------------------------------
//
void CityModel::ConvertCityObject(CityObject& object, rapidjson::Value& id, rapidjson::Value& jobject)
{
    rapidjson::Value jtype;
    rapidjson::Value jgeometry;
    rapidjson::Value attributes;
    
    for (auto& member : jobject.GetObject()) {
        auto memberName = member.name.GetString();
        if (!strcmp(memberName, MEMBER_TYPE)) {
            jtype = member.value;
        }
        else if (!strcmp(memberName, MEMBER_GEOMETRY)) {
            jgeometry = member.value;
        }
        else if (!strcmp(memberName, MEMBER_ATTRIBUTES)) {
            attributes = member.value;
        }
        else if (!strcmp(memberName, MEMBER_PARENTS)) {
            m_converterState.PushMember(memberName);
            for (auto& parent : member.value.GetArray()) {
                object.parents.insert(parent.GetString());
            }
            m_converterState.Pop();
        }
        else if (!strcmp(memberName, MEMBER_CHILDREN)) {
            m_converterState.PushMember(memberName);
            for (auto& child : member.value.GetArray()) {
                object.children.insert(child.GetString());
            }
            m_converterState.Pop();
        }
        else {
            LogMessage(ILog::Level::Info, "Unsupported city object member '%s'", memberName);
        }
    }

    auto type = jtype.GetString();
    //if (!_stricmp(type, "TINRelief"))
    //    return 0;

    OwlInstances geomItems;
    if (jgeometry.IsArray()) {
        m_converterState.PushMember(MEMBER_GEOMETRY);
        m_geometry.Convert(jgeometry, geomItems);
        m_converterState.Pop();
    }

    const char* clsname[] = { type , OWL_ClsCityJSONGenericObject, NULL };
    auto cls = GetOrCreateClass(clsname, true);
    OwlInstance instance = CreateInstance(cls, id.GetString());
    object.owlObject = instance;

    AddNestedObjects(instance, OWL_PropRepresentation, geomItems);

    if (!attributes.IsNull()) {
        m_converterState.PushMember(MEMBER_ATTRIBUTES);
        for (auto& attr : attributes.GetObject()) {
            auto name = attr.name.GetString();
            m_converterState.PushMember(name);
            CreateAttribute(instance, name, OWL_PropAttrPrefix, attr.value);
            m_converterState.Pop();
        }
        m_converterState.Pop();
    }

    CreateAttribute(instance, OWL_PropObjectId, NULL, id);
}

//-----------------------------------------------------------------------------------------------
//
OwlClass CityModel::GetOrCreateClass(const char* names[], bool addPrefix)
{
    if (!names || !names[0]) {
        return NULL;
    }

    const char* name = names[0];
    
    std::string prefixedName;
    if (addPrefix) {
        prefixedName.assign(OWL_ClsCityJsonPrefix);
        prefixedName.append(name);
        name = prefixedName.c_str();
    }

    auto cls = GetClassByName(m_owlModel, name);
    
    if (!cls) {
        cls = CreateClass(m_owlModel, name);

        auto parent = GetOrCreateClass(names + 1, false);
        if (parent) {
            SetClassParent(cls, parent, 1);
        }
    }

    return cls;
}


//-----------------------------------------------------------------------------------------------
//
RdfProperty CityModel::GetOrCreateProperty(OwlClass cls, const char* propName, const char* prefix, RdfPropertyType propType, const char* refCls, int64_t minCard, int64_t maxCard, int attempt)
{
    //std::string fullPropName = GetNameOfClass(cls);
    //fullPropName.append(".");
    //fullPropName.append(propName);
    std::string fullPropName;
    if (prefix) {
        fullPropName.append(prefix);
    }

    fullPropName.append(propName);

    if (attempt > 0) {
        char attmpt[64];
        sprintf(attmpt, "_%d", attempt);
        fullPropName.append(attmpt);
    }

    auto prop = GetPropertyByName(m_owlModel, fullPropName.c_str());
    if (prop) {
        auto ptype = GetPropertyType(prop);
        if (ptype != propType) {
            LogMessage(ILog::Level::Warning, "Porperty '%s' exists but type mismatches", fullPropName.c_str());
            prop = GetOrCreateProperty(cls, propName, NULL, propType, refCls, minCard, maxCard, attempt + 1);
        }
        else {
            int64_t minC = 0;
            int64_t maxC = 0;
            GetClassPropertyCardinalityRestriction(cls, prop, &minC, &maxC);
            if (minC == -1 && maxC == -1) {
                SetClassPropertyCardinalityRestriction(cls, prop, minCard, maxCard);
            }
            else if (minC != minCard || maxC != maxCard) {
                LogMessage(ILog::Level::Warning, "Porperty '%s' exists but cardinality mismatches", fullPropName.c_str());
                prop = GetOrCreateProperty(cls, propName, NULL, propType, refCls, minCard, maxCard, attempt + 1);
            }

        }
    }
    else {
        prop = CreateProperty(m_owlModel, propType, fullPropName.c_str());
        SetClassPropertyCardinalityRestriction(cls, prop, minCard, maxCard);
        if (refCls) {
            auto rc = GetClassByName(RdfModel(), refCls);
            if (rc) {
                SetPropertyRangeRestriction(prop, rc, true);
            }
            else {
                assert(0);
            }
        }
    }

    return prop;
}
//-----------------------------------------------------------------------------------------------
//
OwlInstance CityModel::ConvertAttributeObject(const char* name, rapidjson::Value& value)
{
    const char* clsnames[] = { name, NULL };
    auto cls = GetOrCreateClass(clsnames, true);

    auto inst = CreateInstance(cls);

    for (auto& attr : value.GetObject()) {
        CreateAttribute(inst, attr.name.GetString(), OWL_PropAttrPrefix, attr.value);
    }

    return inst;
}


//-----------------------------------------------------------------------------------------------
//
void CityModel::CreateAttribute(OwlInstance instance, const char* name, const char* prefix, rapidjson::Value& value)
{
    auto cls = GetInstanceClass(instance);
    auto ktype = value.GetType();

    switch (ktype) {
        case rapidjson::kNullType:
            break;

        case rapidjson::kStringType:
        {
            auto val = value.GetString();
            auto prop = GetOrCreateProperty(cls, name, prefix, DATATYPEPROPERTY_TYPE_CHAR, nullptr);
            SetDatatypeProperty(instance, prop, val);
            break;
        }

        case rapidjson::kNumberType:
        {
            auto val = value.GetDouble();
            auto prop = GetOrCreateProperty(cls, name, prefix, DATATYPEPROPERTY_TYPE_DOUBLE, nullptr);
            SetDatatypeProperty(instance, prop, val);
            break;
        }

        case rapidjson::kTrueType:
        case rapidjson::kFalseType:
        {
            auto val = value.GetBool();
            auto prop = GetOrCreateProperty(cls, name, prefix,DATATYPEPROPERTY_TYPE_BOOLEAN, nullptr);
            SetDatatypeProperty(instance, prop, val);
            break;
        }

        case rapidjson::kObjectType:
        {
            auto val = ConvertAttributeObject(name, value);            
            auto prop = GetOrCreateProperty(cls, name, prefix, OBJECTPROPERTY_TYPE, GetNameOfClass(GetInstanceClass(val)));
            SetObjectProperty(instance, prop, val);
            break;
        }

        case rapidjson::kArrayType:
        {
            if (value.Size() > 1) {
                auto kElemType = value[0].GetType();
                switch (kElemType) {
                    case rapidjson::kNumberType:
                    {
                        std::vector<double> val;
                        for (auto& v : value.GetArray()) {
                            val.push_back(v.GetDouble());
                        }
                        auto prop = GetOrCreateProperty(cls, name, prefix, DATATYPEPROPERTY_TYPE_DOUBLE, nullptr, 0, -1);
                        SetDatatypeProperty(instance, prop, val.data(), val.size());
                        break;
                    }

                    default:
                        LogMessage(ILog::Level::Info, "Attribte '%s' array type is not implemented", name);
                }
            }
            break;
        }//case rapidjson::kArrayType

        default:
            LogMessage(ILog::Level::Info, "Attribte '%s' type is not implemented", name);
    }
}
