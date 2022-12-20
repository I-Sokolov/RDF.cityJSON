#pragma once

class CityModel;

class Geometry
{
public:
    Geometry(CityModel& cityModel) : m_cityModel(cityModel) {}

    void SetCityVerticies(rapidjson::Value& verticies);

    void Convert(rapidjson::Value& jgeometry, std::vector<GEOM::GeometricItem>& items);

private:
    typedef std::vector<double>   GeomVertecies;
    typedef std::vector<int64_t>    GeomIndicies;
    typedef std::map<int, int64_t> Vertex2GeomVertex;

private:
    GEOM::GeometricItem ConvertItem(rapidjson::Value& jitem);
    GEOM::GeometricItem ConvertMultiSurface(rapidjson::Value& boundaries);

    void AddListOfSurfaces(rapidjson::Value& jsurfaces, GeomVertecies& vert, GeomIndicies& ind, Vertex2GeomVertex& v2v);
    void AddListOfLoops(rapidjson::Value& jloops, GeomVertecies& vert, GeomIndicies& ind, Vertex2GeomVertex& v2v);
    void AddPoints(rapidjson::Value& jpoints, GeomVertecies& vert, GeomIndicies& ind, Vertex2GeomVertex& v2v);
    int64_t GetAddVertex(rapidjson::Value& jpoint, GeomVertecies& vert, Vertex2GeomVertex& v2v);
    int64_t AddCityVertx(int jcityVertexInd, GeomVertecies& vert);


private:
    CityModel&           m_cityModel;
    rapidjson::Value     m_jcityVerticies;
};

