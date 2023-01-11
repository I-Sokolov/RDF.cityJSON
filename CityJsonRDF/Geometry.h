#pragma once

#include "Semantics.h"
#include "Appearance.h"
class CityModel;

class Geometry
{
public:
    Geometry(CityModel& cityModel) : m_cityModel(cityModel), m_bUseTemplateVerticies (false) {}

    void SetCityVerticies(rapidjson::Value& jverticies) { m_jcityVerticies = jverticies; }

    void SetGeometryTemplates(rapidjson::Value& jtemplates);

    void Convert(rapidjson::Value& jgeometry, OwlInstances& items);

private:
    struct Template
    {
        rapidjson::Value json;
        GEOM::GeometricItem item;
    };

    typedef std::vector<Template> Templates;

    struct PerFaceData
    {
        PerFaceData(CityModel& cityModel) : semantics(cityModel) {}

        rapidjson::Value material;
        rapidjson::Value texture;
        Semantics        semantics;
        UIntList         indexPath;
    };

    struct FaceGroupKey
    {
        OwlInstance             semantic;
        Appearance::Theme2Index materials;
        Appearance::Theme2Index textures;
    };

    struct FaceGroup
    {
        FaceGroupKey        key;
        //        
        DoubleArray         coordinates;
        Int64Array          indecies;
        Int2Int64           cityVert2Coord;

        DoubleArray         texCoordinates;
        Int64Array          texIndecies;
        Int2Int64           texVert2Coord;
    };

    typedef std::list<FaceGroup> FaceGroups;

private:
    GEOM::GeometricItem ConvertItem(rapidjson::Value& jitem);
    GEOM::GeometricItem ConvertCompositeSolid(rapidjson::Value& boundaries, PerFaceData& fd);    
    GEOM::GeometricItem ConvertMultiSolid(rapidjson::Value& boundaries, PerFaceData& fd);
    GEOM::GeometricItem ConvertSolidSet(const char* className, rapidjson::Value& boundaries, PerFaceData& fd);
    GEOM::GeometricItem ConvertSolid(rapidjson::Value& boundaries, PerFaceData& fd);
    GEOM::GeometricItem ConvertCompositeSurface(rapidjson::Value& boundaries, PerFaceData& fd);
    GEOM::GeometricItem ConvertMultiSurface(rapidjson::Value& boundaries, PerFaceData& fd);
    GEOM::GeometricItem ConvertSurfaceSet(const char* className, rapidjson::Value& boundaries, PerFaceData& fd);

    void AddFaceToGroups(FaceGroups& fgroups, rapidjson::Value& boundaries, PerFaceData& fd);
    FaceGroup& GetOrCreateGroup(FaceGroups& fgroups, FaceGroupKey& key);
    bool KeysEqual(FaceGroupKey const& key1, FaceGroupKey& key2);

    void AddFaceToGroup(FaceGroup& faces, rapidjson::Value& boundaries, ListOfListOfInt* texIndecies);

    void AddPoints(rapidjson::Value& jpoints, DoubleArray& coordinates, Int64Array& ind, Int2Int64& v2v);
    int64_t GetAddVertex(rapidjson::Value& jpoint, DoubleArray& coordinates, Int2Int64& v2v);
    int64_t AddVertex(int vertexInd, DoubleArray& coordinates);
    rapidjson::Value& GetVertex(int vertexInd);

    void AddTexturePoints(ListOfInt& jpoints, DoubleArray& coordinates, Int64Array& ind, Int2Int64& v2v);
    int64_t GetAddTextureVertex(int jind, DoubleArray& coordinates, Int2Int64& v2v);
    int64_t AddTextureVertex(int jind, DoubleArray& coordinates);
    rapidjson::Value& GetTextureVertex(int jind);

    GEOM::GeometricItem CreateFaceGroup(FaceGroup& group);

    void UseTemplateVerticies(bool use) { m_bUseTemplateVerticies = use; }

    //GEOM::GeometricItem ConvertFace(rapidjson::Value& jloops, PerFaceData& fd);
    //GEOM::Curve ConvertCurve(rapidjson::Value& jloop);

    GEOM::GeometricItem ConvertGeometryInstance(rapidjson::Value& boundaries, rapidjson::Value& jtemplate, rapidjson::Value& jtransformation);

private:
    CityModel&           m_cityModel;
    rapidjson::Value     m_jcityVerticies;
    rapidjson::Value     m_templateVerticies;
    bool                 m_bUseTemplateVerticies;
    Templates            m_templates;
};

