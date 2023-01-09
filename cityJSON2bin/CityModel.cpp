
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

    CreateBaseClasses();

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
        else if (!strcmp(memberName, MEMBER_METADATA)) {
            metadata = member.value;
        }
        else if (!strcmp(memberName, MEMBER_EXTENSIONS)) {
            extensions = member.value;
        }
        else {
            LOG_CNV("Unsupported cityJSON member", memberName);
        }
    }

    if (!type || strcmp(type, TYPE_CityJSON))
        THROW_ERROR("Expcected type CityJSON");
    if (!version || fabs(atof(version) - VERSION_1_1) > DBL_MIN)
        THROW_ERROR("Unsupported version");

    //
    //
    CityObjects objects;
    int iObject = 0;
    for (auto& o : cityObjects.GetObject()) {
        iObject++;
        //if (iObject != 6)
        //    continue;

        auto& id = o.name;
        auto& cityObject = o.value;

        try {
            CityObject& object = objects[id.GetString()];
            ConvertCityObject(object, id, cityObject);
        }
        catch (cityJson2bin_error err) {
            LOG_CNV("Failed to convert city object", err.c_str());
        }
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
           for (auto& parent : member.value.GetArray()) {
                object.parents.insert(parent.GetString());
            }
        }
        else if (!strcmp(memberName, MEMBER_CHILDREN)) {
            for (auto& child : member.value.GetArray()) {
                object.children.insert(child.GetString());
            }
        }
        else {
            LOG_CNV("Unsupported city object member", memberName);
        }
    }

    auto type = jtype.GetString();
    //if (!_stricmp(type, "TINRelief"))
    //    return 0;

    OwlInstances geomItems;
    if (jgeometry.IsArray()) {
        m_geometry.Convert(jgeometry, geomItems);
    }

    const char* clsname[] = { type , OWL_ClsCityJSONGenericObject, NULL };
    auto cls = GetOrCreateClass(clsname, true);
    OwlInstance instance = CreateInstance(cls, id.GetString());
    object.owlObject = instance;

    AddNestedObjects(instance, OWL_PropRepresentation, geomItems);

    if (!attributes.IsNull()) {
        for (auto& attr : attributes.GetObject()) {
            auto name = attr.name.GetString();
            CreateAttribute(instance, name, OWL_PropAttrPrefix, attr.value);
        }
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
            LOG_CNV("Porperty exists but traits mismatches", fullPropName.c_str());
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
                LOG_CNV("Porperty exists but cardinality mismatches", fullPropName.c_str());
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
                        LOG_CNV("Unsupported attribte array type", name);
                }
            }
            break;
        }//case rapidjson::kArrayType

        default:
            LOG_CNV("Unsupported attribte type", name);
    }
}
