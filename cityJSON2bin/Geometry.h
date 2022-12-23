#pragma once

class CityModel;

class Geometry
{
public:
    Geometry(CityModel& cityModel) : m_cityModel(cityModel) {}

    void SetCityVerticies(rapidjson::Value& verticies);

    void Convert(rapidjson::Value& jgeometry, std::vector<GEOM::GeometricItem>& items);

private:
    typedef std::vector<double>    Coordinates;
    typedef std::vector<int64_t>   GeomIndicies;
    typedef std::map<int, int64_t> Vertex2GeomVertex;

private:
    GEOM::GeometricItem ConvertItem(rapidjson::Value& jitem);
    GEOM::GeometricItem ConvertMultiSurface(rapidjson::Value& boundaries, rapidjson::Value& material, rapidjson::Value& texture);

    void AddListOfSurfaces(rapidjson::Value& jsurfaces, Coordinates& vert, GeomIndicies& ind, Vertex2GeomVertex& v2v);
    void AddListOfLoops(rapidjson::Value& jloops, Coordinates& vert, GeomIndicies& ind, Vertex2GeomVertex& v2v);
    void AddPoints(rapidjson::Value& jpoints, Coordinates& vert, GeomIndicies& ind, Vertex2GeomVertex& v2v);
    int64_t GetAddVertex(rapidjson::Value& jpoint, Coordinates& vert, Vertex2GeomVertex& v2v);
    int64_t AddCityVertx(int jcityVertexInd, Coordinates& vert);

    GEOM::Face ConvertFace(rapidjson::Value& jloops);
    GEOM::Curve ConvertCurve(rapidjson::Value& jloop);

private:
    CityModel&           m_cityModel;
    rapidjson::Value     m_jcityVerticies;
};

