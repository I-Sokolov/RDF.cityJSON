#pragma once

class CityModel;

class Geometry
{
public:
    Geometry(CityModel& cityModel) : m_cityModel(cityModel) {}

    void SetCityVerticies(rapidjson::Value& verticies);

    void Convert(rapidjson::Value& jgeometry);

private:
    typedef std::vector<double>   GeomVertecies;
    typedef std::vector<int_t>    GeomIndicies;
    typedef std::map<int, int_t> Vertex2GeomVertex;

private:
    void ConvertItem(rapidjson::Value& jitem);
    void ConvertMultiSurface(rapidjson::Value& boundaries);

    void AddListOfSurfaces(rapidjson::Value& jsurfaces, GeomVertecies& vert, GeomIndicies& ind, Vertex2GeomVertex& v2v);
    void AddListOfLoops(rapidjson::Value& jloops, GeomVertecies& vert, GeomIndicies& ind, Vertex2GeomVertex& v2v);
    void AddPoints(rapidjson::Value& jpoints, GeomVertecies& vert, GeomIndicies& ind, Vertex2GeomVertex& v2v);
    int_t GetAddVertex(rapidjson::Value& jpoint, GeomVertecies& vert, Vertex2GeomVertex& v2v);
    int_t AddCityVertx(int jcityVertexInd, GeomVertecies& vert);


private:
    CityModel&           m_cityModel;
    rapidjson::Value     m_jcityVerticies;
};

