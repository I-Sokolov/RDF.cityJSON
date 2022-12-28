#include "pch.h"
#include "CommonDefs.h"
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
            attributes.back().name = member.name;
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
    }
}

//-----------------------------------------------------------------------------------------------
//

int64_t Semantics::GetSurfaceSemantic(IntList faceIndexPath)
{
    return 0;
}
