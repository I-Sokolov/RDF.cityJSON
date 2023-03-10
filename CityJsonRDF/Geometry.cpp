#include "pch.h"
#include "CityJsonRDF.h"
using namespace CityJsonRDF;
#include "CommonDefs.h"
#include "CityModel.h"
#include "Appearance.h"
#include "Semantics.h"
#include "Geometry.h"

//-----------------------------------------------------------------------------------------------
//
Geometry::Geometry(CityModel& cityModel)
    : m_cityModel(cityModel)
    , m_bUseTemplateVerticies(false)
{
    for (int i = 0; i < 3; i++) {
        m_cityScale[i] = 1;
        m_cityTranslate[i] = 0;
    }
}

//-----------------------------------------------------------------------------------------------
//
void Geometry::Convert(rapidjson::Value& jgeometry, OwlInstances& items)
{
    //std::vector<std::string, std::list<GEOM::GeometricItem>> lod2items;

    int nitem = 0;
    for (auto& jitem : jgeometry.GetArray()) {

        m_cityModel.State().PushArrayIndex(nitem++);
        
        try {
            auto item = ConvertItem(jitem, nitem, false);
            if (item) {
                items.push_back(item);
            }
        }
        catch (CityModel::Exception) {
            m_cityModel.LogMessage(ILog::Level::Error, "Failed to convert geometry item");
        }

        m_cityModel.State().Pop();
    }
/*
    if (items.empty()) {
        return NULL;
    }
    else if (items.size() == 1) {
        return items.front();
    }
    else {
        auto collection = GEOM::Collection::Create(m_cityModel.RdfModel());
        collection.set_objects(items.data(), items.size());
        return collection;
    }
    */
}

//-----------------------------------------------------------------------------------------------
//
GEOM::GeometricItem Geometry::ConvertItem(rapidjson::Value& jitem, int nItem, bool isTemplate)
{
    GEOM::GeometricItem item;

    //
    //
    const char* type = nullptr;
    rapidjson::Value lod;
    rapidjson::Value boundaries;
    rapidjson::Value jtemplate;
    rapidjson::Value jtransformation;
    PerFaceData      fd(m_cityModel);

    for (auto it = jitem.MemberBegin(); it != jitem.MemberEnd(); it++) {
        const char* memberName = it->name.GetString();

        if (!strcmp(memberName, MEMBER_TYPE)) {
            type = it->value.GetString();
        }
        else if (!strcmp(memberName, MEMBER_LOD)) {
            lod = it->value;
        }
        else if (!strcmp(memberName, MEMBER_BOUNDARIES)) {
            boundaries = it->value;
        }
        else if (!strcmp(memberName, MEMBER_MATERIAL)) {
            fd.material = it->value;
        }
        else if (!strcmp(memberName, MEMBER_TEXTURE)) {
#ifndef IGNORE_TEXTURE
            fd.texture = it->value;
#endif
        }
        else if (!strcmp(memberName, MEMBER_TEMPLATE)) {
            jtemplate = it->value;
        }
        else if (!strcmp(memberName, MEMBER_TRANSFORMATION)) {
            jtransformation = it->value;
        }
        else if (!strcmp(memberName, MEMBER_SEMANTICS)) {
            fd.semantics.Init(it->value);
        }
        else {
            m_cityModel.LogMessage(ILog::Level::Info, "Unsupported geometry item member '%s'", memberName);
        }
    }


    //
    //
    if (!type)
        m_cityModel.ThrowError("Geometry item type is missed");
    if (boundaries.IsNull())
        m_cityModel.ThrowError("Geometry item boundaries are missed");

    //
    //
    UIntList faceIndexPath;

    if (!strcmp(type, TYPE_MultiPoint)) {
        m_cityModel.LogMessage(ILog::Level::Info, "Unsupported geometry type '%s'", type);
    }
    else if (!strcmp(type, TYPE_MultiLineString)) {
        m_cityModel.LogMessage(ILog::Level::Info, "Unsupported geometry type '%s'", type);
    }
    else if (!strcmp(type, TYPE_MultiSurface)) {
        item = ConvertMultiSurface(boundaries, fd);
    }
    else if (!strcmp(type, TYPE_CompositeSurface)) {
        item = ConvertCompositeSurface(boundaries, fd);
    }
    else if (!strcmp(type, TYPE_Solid)) {
        item = ConvertSolid(boundaries, fd);
    }
    else if (!strcmp(type, TYPE_MultiSolid)) {
        item = ConvertMultiSolid(boundaries, fd);
    }
    else if (!strcmp(type, TYPE_CompositeSolid)) {
        item = ConvertCompositeSolid(boundaries, fd);
    }
    else if (!strcmp(type, TYPE_GeometryInstance)) {
        item = ConvertGeometryInstance(boundaries, jtemplate, jtransformation);
    }
    else {
        m_cityModel.LogMessage(ILog::Level::Info, "Unsupported geometry type '%s'", type);
    }

    assert(faceIndexPath.empty());

    if (item) {
        std::string name(isTemplate ? "Template" : "Item");
        char num[256];
        snprintf(num, 255, " %d", nItem);
        name.append(num); 
        if (lod.IsString()) {
            name.append(" LoD ");
            name.append(lod.GetString());
        }
        SetNameOfInstance(item, name.c_str());

        if (!lod.IsNull()) {
            m_cityModel.CreateAttribute(item, CJProp_LOD, NULL, lod);
        }
    }

    return item;
}

//-----------------------------------------------------------------------------------------------
//
GEOM::GeometricItem Geometry::ConvertCompositeSolid(rapidjson::Value& boundaries, PerFaceData& fd)
{
    return ConvertSolidSet(TYPE_CompositeSolid, boundaries, fd);
}

//-----------------------------------------------------------------------------------------------
//
GEOM::GeometricItem Geometry::ConvertMultiSolid(rapidjson::Value& boundaries, PerFaceData& fd)
{
    return ConvertSolidSet(TYPE_MultiSolid, boundaries, fd);
}

//-----------------------------------------------------------------------------------------------
//
GEOM::GeometricItem Geometry::ConvertSolidSet(const char* className, rapidjson::Value& boundaries, PerFaceData& fd)
{
    std::vector<GEOM::GeometricItem> solids;
    fd.indexPath.push_back(0);
    for (auto& jsolid : boundaries.GetArray()) {
        auto solid = ConvertSolid(jsolid, fd);
        if (solid) {
            solids.push_back(solid);
        }
        fd.indexPath.back()++;
    }
    fd.indexPath.pop_back();

    auto cls = m_cityModel.GetOrCreateClass(className, true, CJCls_GeometryBody);
    GEOM::Collection csolid = CreateInstance(cls);
    csolid.set_objects(solids.data(), solids.size());

    return csolid;
}

//-----------------------------------------------------------------------------------------------
//
GEOM::GeometricItem Geometry::ConvertSolid(rapidjson::Value& boundaries, PerFaceData& fd)
{
    std::vector<GEOM::GeometricItem> shells;
    fd.indexPath.push_back(0);
    for (auto& jshell : boundaries.GetArray()) {
        auto shell = ConvertSurfaceSet(TYPE_MultiSurface, jshell, fd);
        if (shell) {
            shells.push_back(shell);
        }
        fd.indexPath.back()++;
    }
    fd.indexPath.pop_back();

    auto cls = m_cityModel.GetOrCreateClass(TYPE_Solid, true, CJCls_GeometryBody);

    GEOM::Collection csolid = CreateInstance(cls);
    csolid.set_objects(shells.data(), shells.size());

    return csolid;
}

//-----------------------------------------------------------------------------------------------
//
GEOM::GeometricItem Geometry::ConvertCompositeSurface(rapidjson::Value& boundaries, PerFaceData& fd)
{
    return ConvertSurfaceSet(TYPE_CompositeSurface, boundaries, fd);
}

//-----------------------------------------------------------------------------------------------
//
GEOM::GeometricItem Geometry::ConvertMultiSurface(rapidjson::Value& boundaries, PerFaceData& fd)
{
    return ConvertSurfaceSet(TYPE_MultiSurface, boundaries, fd);
}

//-----------------------------------------------------------------------------------------------
//
GEOM::GeometricItem Geometry::ConvertSurfaceSet(const char* className, rapidjson::Value& boundaries, PerFaceData& fd)
{
    FaceGroups fgroups;
    
    fd.indexPath.push_back(0);
    for (auto& jface : boundaries.GetArray()) {
        AddFaceToGroups(fgroups, jface, fd);
        fd.indexPath.back()++;
    }
    fd.indexPath.pop_back();

    std::vector<GEOM::GeometricItem> items;
    for (auto& group : fgroups) {
        auto item = CreateFaceGroup(group);
        if (item) {
            items.push_back(item);
        }
    }

    if (items.empty()) {
        return NULL;
    }

    auto cls = m_cityModel.GetOrCreateClass(className, true, CJCls_GeometryBody);

    GEOM::Collection multiSurface = CreateInstance(cls);
    multiSurface.set_objects(items.data(), items.size());

    return multiSurface;
}

//-----------------------------------------------------------------------------------------------
//
void Geometry::AddFaceToGroups(FaceGroups& fgroups, rapidjson::Value& boundaries, PerFaceData& fd)
{
    Appearance::SurfaceAppearance app;
    m_cityModel.GetAppearance().GetSurfaceAppearance(app, fd.material, fd.texture, fd.indexPath);

    FaceGroupKey key;
    std::swap(key.materials, app.materials);
    std::swap(key.textures, app.textures);
    key.semantic = fd.semantics.GetSurfaceSemantic(fd.indexPath);

#ifdef ONLY_SURFACE_SEMANTIC
    if (!key.semantic) {
        return;
    }
#endif

    FaceGroup& group = GetOrCreateGroup(fgroups, key);

    auto texVertIndecies = m_cityModel.GetAppearance().GetTextuteIndecies(group.key.textures, app.textureIndecies);

    AddFaceToGroup(group, boundaries, texVertIndecies);
}

//-----------------------------------------------------------------------------------------------
//
Geometry::FaceGroup& Geometry::GetOrCreateGroup(FaceGroups& fgroups, FaceGroupKey& key)
{
    for (auto& group : fgroups) {
        if (KeysEqual (key, group.key)) {
            return group;
        }
    }

    fgroups.push_back(FaceGroup());
    auto& group = fgroups.back();

    group.key.semantic = key.semantic;
    std::swap(group.key.materials, key.materials);
    std::swap(group.key.textures, key.textures);

    return group;
}

//-----------------------------------------------------------------------------------------------
//
bool Geometry::KeysEqual(FaceGroupKey const& key1, FaceGroupKey& key2)
{
    if (key1.semantic != key2.semantic) {
        return false;
    }
    if (key1.materials != key2.materials) {
        return false;
    }
    if (key1.textures != key2.textures) {
        return false;
    }
    return true;
}

//-----------------------------------------------------------------------------------------------
//
void Geometry::AddFaceToGroup(FaceGroup& faces, rapidjson::Value& boundaries, ListOfListOfInt* texIndecies)
{
    if (texIndecies) {
        if (texIndecies->size() != boundaries.Size()) {
           m_cityModel.LogMessage(ILog::Level::Error, "Texture indecies loops size mismatch number of loops in boundary");
        }
    }

    ListOfListOfInt::iterator itTex;
    if (texIndecies) {
        itTex = texIndecies->begin();
    }

    int end = -1;

    for (auto& jloop : boundaries.GetArray()) {

        AddPoints(jloop, faces.coordinates, faces.indecies, faces.cityVert2Coord);
        faces.indecies.push_back(end);

        if (texIndecies && itTex != texIndecies->end()) {
            ListOfInt& texLoop = *itTex;
            if (texLoop.size() != jloop.Size()) {
                m_cityModel.LogMessage(ILog::Level::Error, "Texture indecies size mismatch number of points in loop");
            }
            AddTexturePoints(texLoop, faces.texCoordinates, faces.texIndecies, faces.texVert2Coord);

            itTex++;
        }
        faces.texIndecies.push_back(end);

        end = -2;
    }
}

//-----------------------------------------------------------------------------------------------
//
void Geometry::AddPoints(rapidjson::Value& jpoints, DoubleArray& coordinates, Int64Array& ind, Int2Int64& v2v)
{
#if 1 //inverse loops
    for (auto k = jpoints.Size(); k > 0; k--) {
        auto i = GetAddVertex(jpoints[k-1], coordinates, v2v);
        ind.push_back(i);
    }
#else
    for (auto& jpoint : jpoints.GetArray()) {
        auto i = GetAddVertex(jpoint, coordinates, v2v);
        ind.push_back(i);
    }
#endif
}

//-----------------------------------------------------------------------------------------------
//
int64_t Geometry::GetAddVertex(rapidjson::Value& jpoint, DoubleArray& coordinates, Int2Int64& v2v)
{
    auto jcityVertexInd = jpoint.GetInt();

    auto it = v2v.insert(Int2Int64::value_type(jcityVertexInd, -1)).first;

    if (it->second < 0) {
        it->second = AddVertex(jcityVertexInd, coordinates);
    }

    return it->second;
}

//-----------------------------------------------------------------------------------------------
//
int64_t Geometry::AddVertex(int vertexInd, DoubleArray& coordinates)
{
    assert(coordinates.size() % 3 == 0);

    double coord[3] = { 0,0,0 };
    GetVertex(coord, vertexInd);

    for (int i = 0; i < 3; i++) {
        coordinates.push_back(coord[i]);
    }

    return coordinates.size() / 3 - 1;
}

//-----------------------------------------------------------------------------------------------
//
void Geometry::AddTexturePoints(ListOfInt& jpoints, DoubleArray& coordinates, Int64Array& ind, Int2Int64& v2v)
{
#if 1 //inverse loops
    for (auto k = jpoints.rbegin(); k!=jpoints.rend(); k++) {
        auto i = GetAddTextureVertex(*k, coordinates, v2v);
        ind.push_back(i);
    }
#else
    for (auto& jpoint : jpoints) {
        auto i = GetAddTextureVertex(jpoint, coordinates, v2v);
        ind.push_back(i);
    }
#endif
}

//-----------------------------------------------------------------------------------------------
//
int64_t Geometry::GetAddTextureVertex(int jind, DoubleArray& coordinates, Int2Int64& v2v)
{
    auto ib = v2v.insert(Int2Int64::value_type(jind, -4));

    if (ib.second) {
        ib.first->second = AddTextureVertex(jind, coordinates);
    }

    return ib.first->second;
}

//-----------------------------------------------------------------------------------------------
//
int64_t Geometry::AddTextureVertex(int jind, DoubleArray& coordinates)
{
    assert(coordinates.size() % 2 == 0);

    double v[2] = { 0,0 };
    try
    {
        auto& jpoint = GetTextureVertex(jind);
        for (int i = 0; i < 2; i++) {
            v[i] = jpoint[i].GetDouble();
        }
        for (auto val : v) {coordinates.push_back(val);
        }
    }
    catch (CityModel::Exception) {
        m_cityModel.LogMessage(ILog::Level::Error, "Invalid texture verex index or coordinates");
        return -3;
    }

    return coordinates.size() / 2 - 1;
}

//-----------------------------------------------------------------------------------------------
//
GEOM::GeometricItem Geometry::CreateFaceGroup(FaceGroup& group)
{
    auto cls = m_cityModel.GetOrCreateClass(CJCls_Surface, false, RDFCls_BoundaryRepresentation);

    auto name = GetNameOfInstance(group.key.semantic);
    GEOM::BoundaryRepresentation face = CreateInstance(cls, name);
    face.set_vertices(group.coordinates.data(), group.coordinates.size());
    face.set_indices(group.indecies.data(), group.indecies.size());

    auto rdfMat = m_cityModel.GetAppearance().GetRdfMaterial (group.key.materials, group.key.textures, group.key.semantic);
    if (rdfMat) {
        face.set_material(rdfMat);
    }

    auto semantic = group.key.semantic;
    if (semantic) {
        auto prop = m_cityModel.GetOrCreateProperty(cls, CJProp_Semantic, NULL, OBJECTPROPERTY_TYPE, CJCls_SurfaceSemantic);
        SetObjectTypeProperty(face, prop, &semantic, 1);
    }

    if (group.texIndecies.size() > 0 && group.texIndecies.size() != group.indecies.size()) {
        m_cityModel.LogMessage(CityJsonRDF::ILog::Level::Error, "Number of texture indecies mismatches number of indecies");
        group.texIndecies.clear();
    }

    bool hasTexInd = false;
    for (auto ind : group.texIndecies) {
        if (ind >= 0) {
            hasTexInd = true;
            break;
        }
    }

    if (hasTexInd) {
        face.set_textureCoordinates(group.texCoordinates);
        face.set_textureIndices(group.texIndecies);
    }

    return face;
}
#if 0
//-----------------------------------------------------------------------------------------------
//
GEOM::Curve Geometry::ConvertCurve(rapidjson::Value& jloop)
{
    DoubleArray coord;
    for (auto& ipoint : jloop.GetArray()) {
        auto i = ipoint.GetInt();
        AddVertx(i, coord);
    }

    auto curve = GEOM::PolyLine3D::Create(m_cityModel.RdfModel());
    curve.set_coordinates(coord);

    return curve;
}
#endif

//-----------------------------------------------------------------------------------------------
//
void Geometry::GetVertex(double coord[3], int vertexInd)
{
    rapidjson::Value* jpt = nullptr;

    if (m_bUseTemplateVerticies)
        jpt = &m_templateVerticies[vertexInd];
    else
        jpt = &m_jcityVerticies[vertexInd];

    for (int i = 0; i < 3; i++) {
        coord[i] = (*jpt)[i].GetDouble();

        if (!m_bUseTemplateVerticies) {
            coord[i] = coord[i] * m_cityScale[i] + m_cityTranslate[i];
        }
    }

}

//-----------------------------------------------------------------------------------------------
//

void Geometry::SetCityTransform(rapidjson::Value& jtransform)
{
    for (auto it = jtransform.MemberBegin(); it != jtransform.MemberEnd(); it++) {
        const char* memberName = it->name.GetString();

        if (!strcmp(memberName, MEMBER_SCALE)) {
            for (int i = 0; i < 3; i++) {
                m_cityScale[i] = (it->value)[i].GetDouble();
            }
        }
        else if (!strcmp(memberName, MEMBER_TRANSLATE)) {
            for (int i = 0; i < 3; i++) {
                m_cityTranslate[i] = (it->value)[i].GetDouble();
            }
        }
        else {
            m_cityModel.LogMessage(ILog::Level::Info, "Unsupported transform member '%s'", memberName);
        }
    }
}

//-----------------------------------------------------------------------------------------------
//
rapidjson::Value& Geometry::GetTextureVertex(int vertexInd)
{
    return m_cityModel.GetAppearance().GetTextureVertex(vertexInd);
}


//-----------------------------------------------------------------------------------------------
//
void Geometry::SetGeometryTemplates(rapidjson::Value& jtemplates)
{
    for (auto& member : jtemplates.GetObject()) {
        auto memberName = member.name.GetString();
        if (!strcmp(memberName, MEMBER_TEMPLATES)) {
            for (auto& jtemplate : member.value.GetArray()) {
                m_templates.push_back(Template());
                m_templates.back().json = jtemplate;
            }
        }
        else if (!strcmp(memberName, MEMBER_TEMPL_VERT)) {
            m_templateVerticies = member.value;
        }
    }
}

//-----------------------------------------------------------------------------------------------
GEOM::GeometricItem Geometry::ConvertGeometryInstance(rapidjson::Value& boundaries, rapidjson::Value& jtemplate, rapidjson::Value& jtransformation)
{
    auto nTemplate = jtemplate.GetInt();
    if (nTemplate < 0 || nTemplate >= m_templates.size()) {
        m_cityModel.ThrowError("Geometry template index %d is out of range", nTemplate);
    }

    auto& tpl = m_templates[nTemplate];
    if (!tpl.json.IsNull()) {
        UseTemplateVerticies(true);
        tpl.item = ConvertItem(tpl.json, nTemplate, true);
        UseTemplateVerticies(false);
        tpl.json.SetNull();
    }

    if (!tpl.item) {
        return NULL;
    }

    double t[12];
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            double v = jtransformation[4*i + j].GetDouble();
            t[4*i + j] = v;
        }
    }

    auto nInsertPt = boundaries[0].GetInt();

    double rInsertPt[3] = { 0,0,0 };
    GetVertex(rInsertPt, nInsertPt);

    for (int i = 0; i < 3; i++){
        double v = rInsertPt[i];
        t[4*i + 3] += v;
    }

    GEOM::Matrix T = GEOM::Matrix::Create(m_cityModel.RdfModel());
    //T.set_coordinates(t, 12);
    T.set__11(t[0]);
    T.set__21(t[1]);
    T.set__31(t[2]);
    T.set__41(t[3]);
    T.set__12(t[4]);
    T.set__22(t[5]);
    T.set__32(t[6]);
    T.set__42(t[7]);
    T.set__13(t[8]);
    T.set__23(t[9]);
    T.set__33(t[10]);
    T.set__43(t[11]);

    auto cls = m_cityModel.GetOrCreateClass(TYPE_GeometryInstance, true, CJCls_GeometryObject, RDFCls_Transformation);
    
    GEOM::Transformation trans = CreateInstance(cls);
    trans.set_object(tpl.item);
    trans.set_matrix(T);

    return trans;
}
