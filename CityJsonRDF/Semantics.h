#pragma once

class CityModel;

class Semantics
{
public: 
    Semantics (CityModel& cityModel) : m_cityModel (cityModel) {}

    void Init(rapidjson::Value& semantics);

    OwlInstance GetSurfaceSemantic(UIntList faceIndexPath);

private:
    struct Attribute
    {
        const char*      name;
        rapidjson::Value value;
    };
    
    typedef std::list<Attribute> Attributes;

    struct Surface
    {
        Surface();
        void Init(rapidjson::Value& jsurf);

        rapidjson::Value type;

        int              parent;
        std::list<int>   children;
        
        Attributes       attributes;

        OwlInstance      owlInstance;
    };

    typedef std::vector<Surface> Surfaces;

private:
    OwlInstance GetOwlInstance(Surface& surf);

private:
    CityModel&           m_cityModel;
    Surfaces             m_surfaces;
    rapidjson::Value     m_values;
};

