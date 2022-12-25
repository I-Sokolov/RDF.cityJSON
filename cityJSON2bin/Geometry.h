#pragma once

class CityModel;

class Geometry
{
public:
    Geometry(CityModel& cityModel) : m_cityModel(cityModel) {}

    void Convert(rapidjson::Value& jgeometry, std::vector<GEOM::GeometricItem>& items);

private:
    typedef std::vector<int64_t>   GeomIndicies;
    typedef std::map<int, int64_t> Vertex2GeomVertex;
    
private:
    GEOM::GeometricItem ConvertItem(rapidjson::Value& jitem);
    GEOM::GeometricItem ConvertCompositeSolid(rapidjson::Value& boundaries, IntList& faceIndexPath, rapidjson::Value& material, rapidjson::Value& texture);    
    GEOM::GeometricItem ConvertMultiSolid(rapidjson::Value& boundaries, IntList& faceIndexPath, rapidjson::Value& material, rapidjson::Value& texture);
    GEOM::GeometricItem ConvertSolidSet(const char* className, rapidjson::Value& boundaries, IntList& faceIndexPath, rapidjson::Value& material, rapidjson::Value& texture);
    GEOM::GeometricItem ConvertSolid(rapidjson::Value& boundaries, IntList& faceIndexPath, rapidjson::Value& material, rapidjson::Value& texture);
    GEOM::GeometricItem ConvertCompositeSurface(rapidjson::Value& boundaries, IntList& faceIndexPath, rapidjson::Value& material, rapidjson::Value& texture);
    GEOM::GeometricItem ConvertMultiSurface(rapidjson::Value& boundaries, IntList& faceIndexPath, rapidjson::Value& material, rapidjson::Value& texture);
    GEOM::GeometricItem ConvertSurfaceSet(const char* className, rapidjson::Value& boundaries, IntList& faceIndexPath, rapidjson::Value& material, rapidjson::Value& texture);

    void AddListOfSurfaces(rapidjson::Value& jsurfaces, DoubleArray& coordinates, GeomIndicies& ind, Vertex2GeomVertex& v2v);
    void AddListOfLoops(rapidjson::Value& jloops, DoubleArray& coordinates, GeomIndicies& ind, Vertex2GeomVertex& v2v);
    void AddPoints(rapidjson::Value& jpoints, DoubleArray& coordinates, GeomIndicies& ind, Vertex2GeomVertex& v2v);
    int64_t GetAddVertex(rapidjson::Value& jpoint, DoubleArray& coordinates, Vertex2GeomVertex& v2v);
    int64_t AddCityVertx(int jcityVertexInd, DoubleArray& coordinates);

    GEOM::GeometricItem ConvertFace(rapidjson::Value& jloops, IntList& faceIndexPath, rapidjson::Value& material, rapidjson::Value& texture);
    GEOM::Curve ConvertCurve(rapidjson::Value& jloop);

private:
    CityModel&           m_cityModel;
};

