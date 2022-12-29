#include "pch.h"
#include "CommonDefs.h"
#include "CityModel.h"
#include "Semantics.h"

//-----------------------------------------------------------------------------------------------
//

Semantics::Surface::Surface()
    : parent(-1)
    , owlInstance(0)
{
}

//-----------------------------------------------------------------------------------------------
//

void Semantics::Surface::Init(rapidjson::Value& jsurf)
{
    for (auto& member : jsurf.GetObject()) {
        auto memberName = member.name.GetString();
        if (!strcmp(memberName, MEMBER_TYPE)) {
            type = member.value;
        }
        else if (!strcmp(memberName, MEMBER_PARENT)) {
            parent = member.value.GetInt();
        }
        else if (!strcmp(memberName, MEMBER_CHILDREN)) {
            for (auto& child : member.value.GetArray()) {
                children.push_back(child.GetInt());
            }
        }
        else {
            attributes.push_back(Attribute());
            attributes.back().name = member.name.GetString();
            attributes.back().value = member.value;
        }
    }
}


//-----------------------------------------------------------------------------------------------
//

void Semantics::Init(rapidjson::Value& semantics)
{
    //parse
    for (auto& member : semantics.GetObject()) {
        auto memberName = member.name.GetString();
        if (!strcmp(memberName, MEMBER_SURFACES)) {
            for (auto& jsurf : member.value.GetArray()) {
                m_surfaces.push_back(Surface());
                m_surfaces.back().Init(jsurf);
            }
        }
        else if (!strcmp(memberName, MEMBER_VALUES)) {
            m_values = member.value;
        }
        else {
            LOG_CNV("Unsupported semantics member", memberName);
        }
    }

    //set parents
    for (int iParent = 0; iParent < m_surfaces.size(); iParent++) {
        
        auto& parent = m_surfaces[iParent];

        for (auto iChild : parent.children) {
            auto& child = m_surfaces[iChild];            
            if (child.parent >= 0 && child.parent != iParent) {
                LOG_CNV("Invalid semantic surface parent", child.type.GetString());
            }
            child.parent = iParent;
        }

        parent.children.clear();
    }
}

//-----------------------------------------------------------------------------------------------
//

int64_t Semantics::GetSurfaceSemantic(UIntList faceIndexPath)
{
    auto values = &m_values;
    for (auto i : faceIndexPath) {
        if (values->IsArray() && values->GetArray().Size() > i)
            values = &((*values)[i]);
    }

    if (!values->IsInt()) {
        return 0;
    }

    int i = values->GetInt();
    if (i < 0 || i >= m_surfaces.size()) {
        LOG_CNV("Semantic surface index is out of range","");
        return 0;
    }

    auto& surf = m_surfaces[i];

    return GetOwlInstance(surf);
}

//-----------------------------------------------------------------------------------------------
//
int64_t Semantics::GetOwlInstance(Surface& surf)
{
    if (!surf.type.IsNull()) {

        const char* clsname[] = { OWL_CityJsonPrefix "SurfaceSemantic", NULL };
        auto cls = m_cityModel.GetOrCreateClass(clsname);
        surf.owlInstance = CreateInstance(cls);

        auto prop = m_cityModel.GetOrCreateProperty(cls, "cityJSON_Type", DATATYPEPROPERTY_TYPE_CHAR);
        const char* type = surf.type.GetString();
        SetDatatypeProperty(surf.owlInstance, prop, type);

        if (surf.parent >= 0 && surf.parent < m_surfaces.size()) {
            auto& parent = m_surfaces[surf.parent];
            auto owlParent = GetOwlInstance(parent);
            if (owlParent) {
                prop = m_cityModel.GetOrCreateProperty(cls, MEMBER_PARENT, OBJECTPROPERTY_TYPE);
                SetObjectProperty(surf.owlInstance, prop, owlParent);
            }
        }

        for (auto& attr : surf.attributes) {
            m_cityModel.CreateAttribute(surf.owlInstance, attr.name, attr.value);
        }

        surf.attributes.clear();
        surf.type.SetNull();
    }

    return surf.owlInstance;
}