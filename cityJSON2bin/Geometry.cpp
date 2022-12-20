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
void Geometry::SetCityVerticies(rapidjson::Value& jverticies)
{
    assert(jverticies.IsArray());
    m_jcityVerticies = jverticies;
    assert(jverticies.IsNull());
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
    GeomVertecies vert;
    GeomIndicies  ind;
    Vertex2GeomVertex v2v;
    AddListOfSurfaces(boundaries, vert, ind, v2v);

   // auto cls = m_cityModel.GetOrCreateClass(TYPE_MultiSurface, OWL_BoundaryRepresentation);
   // GEOM::BoundaryRepresentation ms = CreateInstance(cls);

}

//-----------------------------------------------------------------------------------------------
//
void Geometry::AddListOfSurfaces(rapidjson::Value& jsurfaces, GeomVertecies& vert, GeomIndicies& ind, Vertex2GeomVertex& v2v)
{
    for (auto& loops : jsurfaces.GetArray()) {
        AddListOfLoops(loops, vert, ind, v2v);
    }
}

//-----------------------------------------------------------------------------------------------
//
void Geometry::AddListOfLoops(rapidjson::Value& jloops, GeomVertecies& vert, GeomIndicies& ind, Vertex2GeomVertex& v2v)
{
    int end = -1;
    for (auto& jloop : jloops.GetArray()) {
        AddPoints(jloop, vert, ind, v2v);
        ind.push_back(end);
        end = -2;
    }
}

//-----------------------------------------------------------------------------------------------
//
void Geometry::AddPoints(rapidjson::Value& jpoints, GeomVertecies& vert, GeomIndicies& ind, Vertex2GeomVertex& v2v)
{
    for (auto& jpoint : jpoints.GetArray()) {
        auto i = GetAddVertex(jpoint, vert, v2v);
        ind.push_back(i);
    }
}

//-----------------------------------------------------------------------------------------------
//
int_t Geometry::GetAddVertex(rapidjson::Value& jpoint, GeomVertecies& vert, Vertex2GeomVertex& v2v)
{
    auto jcityVertexInd = jpoint.GetInt();
    
    auto it = v2v.insert(Vertex2GeomVertex::value_type(jcityVertexInd, -1)).first;

    if (it->second < 0) {
        it->second = AddCityVertx(jcityVertexInd, vert);
    }
    
    return it->second;
}

//-----------------------------------------------------------------------------------------------
//
int_t Geometry::AddCityVertx(int jcityVertexInd, GeomVertecies& vert)
{
    assert(vert.size() % 3 == 0);

    auto& jpoint = m_jcityVerticies[jcityVertexInd];
    for (int i = 0; i < 3; i++) {
        auto v = jpoint[i].GetDouble();
        vert.push_back(v);
    }

    return vert.size() / 3 - 1;
}
