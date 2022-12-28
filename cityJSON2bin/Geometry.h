#pragma once

class CityModel;

class Geometry
{
public:
    Geometry(CityModel& cityModel) : m_cityModel(cityModel), m_bUseTemplateVerticies (false) {}

    void SetCityVerticies(rapidjson::Value& jverticies) { m_jcityVerticies = jverticies; }
    rapidjson::Value& GetVertex(int vertexInd);

    void SetGeometryTemplates(rapidjson::Value& jtemplates);

    void Convert(rapidjson::Value& jgeometry, std::vector<GEOM::GeometricItem>& items);


private:
    typedef std::vector<int64_t>   GeomIndicies;
    typedef std::map<int, int64_t> Vertex2GeomVertex;
    
    struct Template
    {
        rapidjson::Value json;
        GEOM::GeometricItem item;
    };

    typedef std::vector<Template> Templates;

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
    int64_t AddVertx(int vertexInd, DoubleArray& coordinates);

    void UseTemplateVerticies(bool use) { m_bUseTemplateVerticies = use; }

    GEOM::GeometricItem ConvertFace(rapidjson::Value& jloops, IntList& faceIndexPath, rapidjson::Value& material, rapidjson::Value& texture);
    GEOM::Curve ConvertCurve(rapidjson::Value& jloop);

    GEOM::GeometricItem ConvertGeometryInstance(rapidjson::Value& boundaries, rapidjson::Value& jtemplate, rapidjson::Value& jtransformation);

private:
    CityModel&           m_cityModel;
    rapidjson::Value     m_jcityVerticies;
    rapidjson::Value     m_templateVerticies;
    bool                 m_bUseTemplateVerticies;
    Templates            m_templates;
};

