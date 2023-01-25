#pragma once

//====================================================================

#define VERSION_1_1         1.1

#define MEMBER_TYPE             "type"
#define MEMBER_VERSION          "version"
#define MEMBER_TRANSFORM        "transform"
#define MEMBER_VERTICIES        "vertices"
#define MEMBER_APPEARANCE       "appearance"
#define MEMBER_MATERIALS        "materials"
#define MEMBER_NAME             "name"
#define MEMBER_AMBIENT_INTENSITY "ambientIntensity"
#define MEMBER_DIFFUSE_COLOR    "diffuseColor"
#define MEMBER_EMMISSIVE_COLOR  "emissiveColor"
#define MEMBER_SPECULAR_COLOR   "specularColor"
#define MEMBER_SHININESS        "shininess"
#define MEMBER_TRANSPARENCY     "transparency"
#define MEMBER_SMOOTH           "isSmooth"
#define MEMBER_TEXTURES         "textures"
#define MEMBER_IMAGE            "image"
#define MEMBER_TXT_VERTICES     "vertices-texture"
#define MEMBER_DEF_TXT_THEME    "default-theme-texture"
#define MEMBER_DEF_MAT_THEME    "default-theme-material"
#define MEMBER_GEOM_TEMPLATES   "geometry-templates"
#define MEMBER_CITYOBJECTS      "CityObjects"
#define MEMBER_GEOMETRY         "geometry"
#define MEMBER_BOUNDARIES       "boundaries"
#define MEMBER_TEMPLATE         "template"
#define MEMBER_TEMPLATES        "templates"
#define MEMBER_TEMPL_VERT       "vertices-templates"
#define MEMBER_TRANSFORMATION   "transformationMatrix"
#define MEMBER_LOD              "lod"
#define MEMBER_MATERIAL         "material"
#define MEMBER_TEXTURE          "texture"
#define MEMBER_VALUES           "values"
#define MEMBER_VALUE            "value"
#define MEMBER_SEMANTICS        "semantics"
#define MEMBER_SURFACES         "surfaces"
#define MEMBER_PARENT           "parent"
#define MEMBER_PARENTS          "parents"
#define MEMBER_CHILDREN         "children"
#define MEMBER_ATTRIBUTES       "attributes"
#define MEMBER_METADATA         "metadata"
#define MEMBER_EXTENSIONS       "extensions"
#define MEMBER_SCALE            "scale"
#define MEMBER_TRANSLATE        "translate"

#define TYPE_CityJSON           "CityJSON"
#define TYPE_MultiPoint         "MultiPoint"
#define TYPE_MultiLineString    "MultiLineString"
#define TYPE_MultiSurface       "MultiSurface"
#define TYPE_CompositeSurface   "CompositeSurface"
#define TYPE_Solid              "Solid"
#define TYPE_MultiSolid         "MultiSolid"
#define TYPE_CompositeSolid     "CompositeSolid"
#define TYPE_GeometryInstance   "GeometryInstance"

#define RDFCls_GeometricItem            "GeometricItem"
#define RDFCls_BoundaryRepresentation   "BoundaryRepresentation"
#define RDFCls_Collection               "Collection"
#define RDFCls_Transformation           "Transformation"

#define CJCls_Prefix                    "City."
#define CJCls_GenericObject             CJCls_Prefix "Object"
#define CJCls_GeometryObject            CJCls_Prefix "GeometryObject"
#define CJCls_GeometryBody              CJCls_Prefix "GeometryBody"
#define CJCls_SurfaceSemantic           CJCls_Prefix "SurfaceSemantic"
#define CJCls_Surface                   CJCls_Prefix "Surface"
#define CJCls_Attributes                CJCls_Prefix "Attributes"

#define CJProp_Prefix                   "city.prop."
#define CJProp_Representation           "representation"
#define CJProp_Children                 "children"
#define CJProp_ObjectId                 CJProp_Prefix "id"
#define CJProp_Type                     CJProp_Prefix "type"
#define CJProp_LOD                      CJProp_Prefix "LoD"
#define CJProp_Semantic                 CJProp_Prefix "semantic"
#define CJProp_AttrPrefix               CJProp_Prefix "attr."

//====================================================================

typedef std::list<unsigned int>     UIntList;
typedef std::vector<double>         DoubleArray;
typedef std::vector<int64_t>        Int64Array;
typedef std::map<int,int64_t>       Int2Int64;
typedef std::list<int>              ListOfInt;
typedef std::list<ListOfInt>        ListOfListOfInt;
typedef std::set<std::string>       ListOfString;
typedef std::vector<OwlInstance>    OwlInstances;

//====================================================================

#ifdef _DEBUG
    #define TRACE_CNV printf
#else 
    #define TRACE_CNV __noop
#endif // DEBUG

//====================================================================
extern bool JsonReadFile(rapidjson::Document& doc, const char* filePath);

//====================================================================

class ConverterState
{
public:
    ~ConverterState()
    {
        assert(m_state.empty());
    }

    void PushMember(const char* name)
    {
        m_state.push_back(StateItem());
        m_state.back().isMember = true;
        m_state.back().name = name;
    }

    void PushArrayIndex(int index)
    {
        m_state.push_back(StateItem());
        m_state.back().i = index;
    }

    void Pop()
    {
        m_state.pop_back();
    }

    std::string ToString() 
    {
        std::string path;
        for (auto& item : m_state)
        {
            if (item.isMember)
            {
                path.append("/");
                path.append(item.name);
            }
            else
            {
                char ind[80];
                sprintf(ind, "[%d]", item.i);
                path.append(ind);
            }
        }

        return path; 
    }

private:
    struct StateItem
    {
        bool        isMember = false;
        std::string name;
        int         i = 0;
    };

    std::list<StateItem> m_state;
};

