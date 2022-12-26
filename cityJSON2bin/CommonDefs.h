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
#define MEMBER_CITYOBJECTS      "CityObjects"
#define MEMBER_GEOMETRY         "geometry"
#define MEMBER_BOUNDARIES       "boundaries"
#define MEMBER_LOD              "lod"
#define MEMBER_SEMANTICS        "semantics"
#define MEMBER_MATERIAL         "material"
#define MEMBER_TEXTURE          "texture"
#define MEMBER_VALUES           "values"
#define MEMBER_VALUE            "value"

#define TYPE_CityJSON           "CityJSON"
#define TYPE_MultiPoint         "MultiPoint"
#define TYPE_MultiLineString    "MultiLineString"
#define TYPE_MultiSurface       "MultiSurface"
#define TYPE_CompositeSurface   "CompositeSurface"
#define TYPE_Solid              "Solid"
#define TYPE_MultiSolid         "MultiSolid"
#define TYPE_CompositeSolid     "CompositeSolid"
#define TYPE_GeometryInstance   "GeometryInstance"

#define OWL_CityJsonPrefix              "cityJSON."
#define OWL_BoundaryRepresentation      "BoundaryRepresentation"
#define OWL_Collection                  "Collection"

//====================================================================

typedef std::list<int>         IntList;
typedef std::vector<double>    DoubleArray;

//====================================================================

#ifdef _DEBUG
    #define TRACE_CNV printf
#else 
    #define TRACE_CNV __noop
#endif // DEBUG

//====================================================================

extern void LOG_CNV(const char* catergory, const char* msg);

//====================================================================

extern void THROW_ERROR(const char* error_code);


