#include "pch.h"
#include "CommonDefs.h"
#include "Geometry.h"

//-----------------------------------------------------------------------------------------------
//
void Geometry::Convert(rapidjson::Value& jgeometry)
{
    for (auto& item : jgeometry.GetArray()) {
        ConvertItem(item);
    }
}

//-----------------------------------------------------------------------------------------------
//
void Geometry::ConvertItem(rapidjson::Value& jitem)
{
    auto type = jitem[MEMBER_TYPE].GetString();
    auto& boundaries = jitem[MEMBER_BOUNDARIES];

    if (!strcmp(type, TYPE_MultiPoint)) {
        //THROW_ERROR("Unsupported geometry type");
    }
    else if (!strcmp(type, TYPE_MultiLineString)) {
        //THROW_ERROR("Unsupported geometry type");
    }
    else if (!strcmp(type, TYPE_MultiSurface)) {
        ConvertMultiSurface (boundaries);
    }
    else if (!strcmp(type, TYPE_CompositeSurface)) {
        //THROW_ERROR("Unsupported geometry type");
    }
    else if (!strcmp(type, TYPE_Solid)) {
        //THROW_ERROR("Unsupported geometry type");
    }
    else if (!strcmp(type, TYPE_MultiSolid)) {
        //THROW_ERROR("Unsupported geometry type");
    }
    else if (!strcmp(type, TYPE_CompositeSolid)) {
        //THROW_ERROR("Unsupported geometry type");
    }
    else if (!strcmp(type, TYPE_GeometryInstance)) {
        //THROW_ERROR("Unsupported geometry type");
    }
    else {
        //THROW_ERROR("Unknown geometry type");
    }
}

//-----------------------------------------------------------------------------------------------
//
void Geometry::ConvertMultiSurface(rapidjson::Value& boundaries)
{
    for (auto& bnd : boundaries.GetArray()) {
        ConvertSurface(bnd);
    }
}

//-----------------------------------------------------------------------------------------------
//
void Geometry::ConvertSurface(rapidjson::Value& boundaries)
{
    bool first = true;
    for (auto& jloop : boundaries.GetArray()) {
        Loop loop;
        GetLoop(loop, jloop);
    }
}

//-----------------------------------------------------------------------------------------------
//
void Geometry::GetLoop(Loop& loop, rapidjson::Value& jloop)
{
    printf("loop: ");
    for (auto& jpoint : jloop.GetArray()) {
        loop.push_back(Point3D());
        GetPoint(loop.back(), jpoint);
    }
    printf("\n");
}

//-----------------------------------------------------------------------------------------------
//
void Geometry::GetPoint(Point3D& point, rapidjson::Value& jpoint)
{
    auto ind = jpoint.GetInt();
    printf(" %d", ind);
    for (int i = 0; i < 3; i++) {
        point.coord[i] = 0;
    }
}
