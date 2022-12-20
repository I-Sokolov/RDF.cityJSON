#pragma once

//====================================================================

#define VERSION_1_1         1.1

#define MEMBER_TYPE         "type"
#define MEMBER_VERSION      "version"
#define MEMBER_TRANSFORM    "transform"
#define MEMBER_VERTICIES    "vertices"
#define MEMBER_CITYOBJECTS  "CityObjects"
#define MEMBER_GEOMETRY     "geometry"
#define MEMBER_BOUNDARIES   "boundaries"

#define TYPE_CityJSON           "CityJSON"
#define TYPE_MultiPoint         "MultiPoint"
#define TYPE_MultiLineString    "MultiLineString"
#define TYPE_MultiSurface       "MultiSurface"
#define TYPE_CompositeSurface   "CompositeSurface"
#define TYPE_Solid              "Solid"
#define TYPE_MultiSolid         "MultiSolid"
#define TYPE_CompositeSolid     "CompositeSolid"
#define TYPE_GeometryInstance   "GeometryInstance"

#define OWL_BoundaryRepresentation      "BoundaryRepresentation"
#define OWL_Collection                  "Collection"

//====================================================================

extern void THROW_ERROR(const char* error_code);
