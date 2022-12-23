#include "pch.h"
#include "CommonDefs.h"
#include "CityModel.h"
#include "Geometry.h"

//-----------------------------------------------------------------------------------------------
//
void Geometry::Convert(rapidjson::Value& jgeometry, std::vector< GEOM::GeometricItem>& items)
{
    //std::vector<std::string, std::list<GEOM::GeometricItem>> lod2items;

    for (auto& jitem : jgeometry.GetArray()) {
        auto item = ConvertItem(jitem);
        if (item) {
            items.push_back(item);
        }
    }
/*
    if (items.empty()) {
        return NULL;
    }
    else if (items.size() == 1) {
        return items.front();
    }
    else {
        auto collection = GEOM::Collection::Create(m_cityModel.GetModel());
        collection.set_objects(items.data(), items.size());
        return collection;
    }
    */
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
GEOM::GeometricItem Geometry::ConvertItem(rapidjson::Value& jitem)
{
    auto type = jitem[MEMBER_TYPE].GetString();
    auto& boundaries = jitem[MEMBER_BOUNDARIES];

    if (!strcmp(type, TYPE_MultiPoint)) {
        TRACE("Unsupported geometry type: %s\n", type);
    }
    else if (!strcmp(type, TYPE_MultiLineString)) {
        TRACE("Unsupported geometry type: %s\n", type);
    }
    else if (!strcmp(type, TYPE_MultiSurface)) {
        return ConvertMultiSurface (boundaries);
    }
    else if (!strcmp(type, TYPE_CompositeSurface)) {
        TRACE("Unsupported geometry type: %s\n", type);
    }
    else if (!strcmp(type, TYPE_Solid)) {
        TRACE("Unsupported geometry type: %s\n", type);
    }
    else if (!strcmp(type, TYPE_MultiSolid)) {
        TRACE("Unsupported geometry type: %s\n", type);
    }
    else if (!strcmp(type, TYPE_CompositeSolid)) {
        TRACE("Unsupported geometry type: %s\n", type);
    }
    else if (!strcmp(type, TYPE_GeometryInstance)) {
        TRACE("Unsupported geometry type: %s\n", type);
    }
    else {
        TRACE("Unsupported geometry type: %s\n", type);
    }

    return NULL;
}

//-----------------------------------------------------------------------------------------------
//
GEOM::GeometricItem Geometry::ConvertMultiSurface(rapidjson::Value& boundaries)
{
    GeomVertecies vert;
    GeomIndicies  ind;
    Vertex2GeomVertex v2v;
    AddListOfSurfaces(boundaries, vert, ind, v2v);

    const char* clsnames[] = { TYPE_MultiSurface, OWL_BoundaryRepresentation, NULL };
    auto cls = m_cityModel.GetOrCreateClass(clsnames);
   
   GEOM::BoundaryRepresentation multiSurface = CreateInstance(cls);
   if (!multiSurface) {
       THROW_ERROR("Failed to create " TYPE_MultiSurface " instance");
   }

   multiSurface.set_vertices(vert.data(), vert.size());
   multiSurface.set_indices(ind.data(), ind.size());

   return multiSurface;
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
int64_t Geometry::GetAddVertex(rapidjson::Value& jpoint, GeomVertecies& vert, Vertex2GeomVertex& v2v)
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
int64_t Geometry::AddCityVertx(int jcityVertexInd, GeomVertecies& vert)
{
    assert(vert.size() % 3 == 0);

    auto& jpoint = m_jcityVerticies[jcityVertexInd];
    for (int i = 0; i < 3; i++) {
        auto v = jpoint[i].GetDouble();
        vert.push_back(v);
    }

    return vert.size() / 3 - 1;
}
