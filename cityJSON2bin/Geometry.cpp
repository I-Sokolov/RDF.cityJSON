#include "pch.h"
#include "cityJson2bin.h"
#include "CommonDefs.h"
#include "CityModel.h"
#include "Appearance.h"
#include "Geometry.h"

//-----------------------------------------------------------------------------------------------
//
void Geometry::Convert(rapidjson::Value& jgeometry, std::vector<GEOM::GeometricItem>& items)
{
    //std::vector<std::string, std::list<GEOM::GeometricItem>> lod2items;

    for (auto& jitem : jgeometry.GetArray()) {
        //auto type = jitem[MEMBER_TYPE].GetString();
        //if (strcmp(type, TYPE_GeometryInstance))
        //    continue;

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
        auto collection = GEOM::Collection::Create(m_cityModel.RdfModel());
        collection.set_objects(items.data(), items.size());
        return collection;
    }
    */
}

//-----------------------------------------------------------------------------------------------
//
GEOM::GeometricItem Geometry::ConvertItem(rapidjson::Value& jitem)
{
    GEOM::GeometricItem item;

    try {
        //
        //
        const char* type = nullptr;
        const char* lod = nullptr;
        rapidjson::Value boundaries;
        rapidjson::Value semantics;
        rapidjson::Value material;
        rapidjson::Value texture;
        rapidjson::Value jtemplate;
        rapidjson::Value jtransformation;

        for (auto it = jitem.MemberBegin(); it != jitem.MemberEnd(); it++) {
            const char* memberName = it->name.GetString();

            if (!strcmp(memberName, MEMBER_TYPE)) {
                type = it->value.GetString();
            }
            else if (!strcmp(memberName, MEMBER_LOD)) {
                lod = it->value.GetString();
            }
            else if (!strcmp(memberName, MEMBER_BOUNDARIES)) {
                boundaries = it->value;
            }
            else if (!strcmp(memberName, MEMBER_SEMANTICS)) {
                semantics = it->value;
            }
            else if (!strcmp(memberName, MEMBER_MATERIAL)) {
                material = it->value;
            }
            else if (!strcmp(memberName, MEMBER_TEXTURE)) {
                texture = it->value;
            }
            else if (!strcmp(memberName, MEMBER_TEMPLATE)) {
                jtemplate = it->value;
            }
            else if (!strcmp(memberName, MEMBER_TRANSFORMATION)) {
                jtransformation = it->value;
            }
            else {
                LOG_CNV("Unsupported geometry item member", memberName);
            }
        }


        //
        //
        if (!type)
            THROW_ERROR("Geometry item type is missed");
        if (boundaries.IsNull())
            THROW_ERROR("Geometry item boundaries are missed");

        //
        //
        IntList faceIndexPath;

        if (!strcmp(type, TYPE_MultiPoint)) {
            LOG_CNV("Unsupported geometry type", type);
        }
        else if (!strcmp(type, TYPE_MultiLineString)) {
            LOG_CNV("Unsupported geometry type", type);
        }
        else if (!strcmp(type, TYPE_MultiSurface)) {
            item = ConvertMultiSurface(boundaries, faceIndexPath, material, texture);
        }
        else if (!strcmp(type, TYPE_CompositeSurface)) {
            item = ConvertCompositeSurface(boundaries, faceIndexPath, material, texture);
        }
        else if (!strcmp(type, TYPE_Solid)) {
            item = ConvertSolid(boundaries, faceIndexPath, material, texture);
        }
        else if (!strcmp(type, TYPE_MultiSolid)) {
            item = ConvertMultiSolid(boundaries, faceIndexPath, material, texture);
        }
        else if (!strcmp(type, TYPE_CompositeSolid)) {
            item = ConvertCompositeSolid(boundaries, faceIndexPath, material, texture);
        }
        else if (!strcmp(type, TYPE_GeometryInstance)) {
            item = ConvertGeometryInstance(boundaries, jtemplate, jtransformation);
        }
        else {
            LOG_CNV("Unsupported geometry type", type);
        }

        assert(faceIndexPath.empty());
    }
    catch (cityJson2bin_error err) {
        item = 0;
        LOG_CNV("Failed to convert geometry item", err.c_str());
    }

    return item;
}

//-----------------------------------------------------------------------------------------------
//
GEOM::GeometricItem Geometry::ConvertCompositeSolid(rapidjson::Value& boundaries, IntList& faceIndexPath, rapidjson::Value& material, rapidjson::Value& texture)
{
    return ConvertSolidSet(OWL_CityJsonPrefix TYPE_CompositeSolid, boundaries, faceIndexPath, material, texture);
}

//-----------------------------------------------------------------------------------------------
//
GEOM::GeometricItem Geometry::ConvertMultiSolid(rapidjson::Value& boundaries, IntList& faceIndexPath, rapidjson::Value& material, rapidjson::Value& texture)
{
    return ConvertSolidSet(OWL_CityJsonPrefix TYPE_MultiSolid, boundaries,faceIndexPath, material, texture);
}

//-----------------------------------------------------------------------------------------------
//
GEOM::GeometricItem Geometry::ConvertSolidSet(const char* className, rapidjson::Value& boundaries, IntList& faceIndexPath, rapidjson::Value& material, rapidjson::Value& texture)
{
    const char* clsnames[] = {className , OWL_Collection, NULL };
    auto cls = m_cityModel.GetOrCreateClass(clsnames);

    std::vector<GEOM::GeometricItem> solids;
    faceIndexPath.push_back(0);
    for (auto& jsolid : boundaries.GetArray()) {
        auto solid = ConvertSolid(jsolid, faceIndexPath, material, texture);
        if (solid) {
            solids.push_back(solid);
        }
        faceIndexPath.back()++;
    }
    faceIndexPath.pop_back();

    GEOM::Collection csolid = CreateInstance(cls);
    csolid.set_objects(solids.data(), solids.size());

    return csolid;
}

//-----------------------------------------------------------------------------------------------
//
GEOM::GeometricItem Geometry::ConvertSolid(rapidjson::Value& boundaries, IntList& faceIndexPath, rapidjson::Value& material, rapidjson::Value& texture)
{
    const char* clsnames[] = { OWL_CityJsonPrefix TYPE_Solid, OWL_Collection, NULL };
    auto cls = m_cityModel.GetOrCreateClass(clsnames);

    std::vector<GEOM::GeometricItem> shells;
    faceIndexPath.push_back(0);
    for (auto& jshell : boundaries.GetArray()) {
        auto shell = ConvertSurfaceSet(TYPE_MultiSurface, jshell, faceIndexPath, material, texture);
        if (shell) {
            shells.push_back(shell);
        }
        faceIndexPath.back()++;
    }
    faceIndexPath.pop_back();

    GEOM::Collection csolid = CreateInstance(cls);
    csolid.set_objects(shells.data(), shells.size());

    return csolid;
}

//-----------------------------------------------------------------------------------------------
//
GEOM::GeometricItem Geometry::ConvertCompositeSurface(rapidjson::Value& boundaries, IntList& faceIndexPath, rapidjson::Value& material, rapidjson::Value& texture)
{
    return ConvertSurfaceSet(OWL_CityJsonPrefix TYPE_CompositeSurface, boundaries, faceIndexPath, material, texture);
}

//-----------------------------------------------------------------------------------------------
//
GEOM::GeometricItem Geometry::ConvertMultiSurface(rapidjson::Value& boundaries, IntList& faceIndexPath, rapidjson::Value& material, rapidjson::Value& texture)
{
    return ConvertSurfaceSet(OWL_CityJsonPrefix TYPE_MultiSurface, boundaries, faceIndexPath, material, texture);
}

//-----------------------------------------------------------------------------------------------
//
GEOM::GeometricItem Geometry::ConvertSurfaceSet(const char* className, rapidjson::Value& boundaries, IntList& faceIndexPath, rapidjson::Value& material, rapidjson::Value& texture)
{
    const char* clsnames[] = { className , OWL_Collection /*OWL_BoundaryRepresentation*/, NULL };
    auto cls = m_cityModel.GetOrCreateClass(clsnames);

    //GEOM::BoundaryRepresentation multiSurface = CreateInstance(cls);
    GEOM::Collection multiSurface = CreateInstance(cls);
    if (!multiSurface) {
        THROW_ERROR("Failed to create " TYPE_MultiSurface " instance");
    }

#if 1
    //faces
    std::vector<GEOM::GeometricItem> faces;
    faceIndexPath.push_back(0);
    for (auto& jface : boundaries.GetArray()) {
        auto face = ConvertFace(jface, faceIndexPath, material, texture);
        if (face) {
            faces.push_back(face);
        }
        faceIndexPath.back()++;
    }
    faceIndexPath.pop_back();

    multiSurface.set_objects(faces.data(), faces.size());
    //multiSurface.set_faces(faces.data(), faces.size());
#else
    //mesh
    Coordinates vert;
    GeomIndicies  ind;
    Vertex2GeomVertex v2v;
    AddListOfSurfaces(boundaries, vert, ind, v2v);

    multiSurface.set_vertices(vert.data(), vert.size());
    multiSurface.set_indices(ind.data(), ind.size());
#endif

    return multiSurface;
}

//-----------------------------------------------------------------------------------------------
//
GEOM::GeometricItem Geometry::ConvertFace(rapidjson::Value& jloops, IntList& faceIndexPath, rapidjson::Value& material, rapidjson::Value& texture)
{
    //mesh
    DoubleArray vert;
    GeomIndicies  ind;
    Vertex2GeomVertex v2v;
    AddListOfLoops(jloops, vert, ind, v2v);

    const char* clsnames[] = { OWL_CityJsonPrefix  "Surface", OWL_BoundaryRepresentation, NULL };
    auto cls = m_cityModel.GetOrCreateClass(clsnames);

    GEOM::BoundaryRepresentation face = CreateInstance(cls);
    face.set_vertices(vert.data(), vert.size());
    face.set_indices(ind.data(), ind.size());

#if 0
    std::vector<GEOM::Curve> curves;
    for (auto& jloop : jloops.GetArray()) {
        auto curve = ConvertCurve(jloop);
        if (curve) {
            curves.push_back(curve);
        }
    }

    if (curves.size() < 1) {
        TRACE_CNV("Empty face\n");
        return NULL;
    }

    auto rcurves = curves.data();
    auto ncurves = curves.size();

    auto face = GEOM::Face2D::Create(m_cityModel.RdfModel());
    face.set_outerPolygon(rcurves[0]);

    if (ncurves > 1) {
        face.set_innerPolygons(rcurves + 1, ncurves - 1);
    }

    return face;
#endif

    auto rdfMat = m_cityModel.GetAppearance().GetFaceMaterial(material, texture, faceIndexPath, jloops);
    if (rdfMat) {
        face.set_material(rdfMat);
    }

    return face;
}

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

//-----------------------------------------------------------------------------------------------
//
void Geometry::AddListOfSurfaces(rapidjson::Value& jsurfaces, DoubleArray& coordinates, GeomIndicies& ind, Vertex2GeomVertex& v2v)
{
    for (auto& loops : jsurfaces.GetArray()) {
        AddListOfLoops(loops, coordinates, ind, v2v);
    }
}

//-----------------------------------------------------------------------------------------------
//
void Geometry::AddListOfLoops(rapidjson::Value& jloops, DoubleArray& coordinates, GeomIndicies& ind, Vertex2GeomVertex& v2v)
{
    int end = -1;
    for (auto& jloop : jloops.GetArray()) {
        AddPoints(jloop, coordinates, ind, v2v);
        ind.push_back(end);
        end = -2;
    }
}

//-----------------------------------------------------------------------------------------------
//
void Geometry::AddPoints(rapidjson::Value& jpoints, DoubleArray& coordinates, GeomIndicies& ind, Vertex2GeomVertex& v2v)
{
    for (auto& jpoint : jpoints.GetArray()) {
        auto i = GetAddVertex(jpoint, coordinates, v2v);
        ind.push_back(i);
    }
}

//-----------------------------------------------------------------------------------------------
//
int64_t Geometry::GetAddVertex(rapidjson::Value& jpoint, DoubleArray& coordinates, Vertex2GeomVertex& v2v)
{
    auto jcityVertexInd = jpoint.GetInt();
    
    auto it = v2v.insert(Vertex2GeomVertex::value_type(jcityVertexInd, -1)).first;

    if (it->second < 0) {
        it->second = AddVertx(jcityVertexInd, coordinates);
    }
    
    return it->second;
}

//-----------------------------------------------------------------------------------------------
//
int64_t Geometry::AddVertx(int vertexInd, DoubleArray& coordinates)
{
    assert(coordinates.size() % 3 == 0);

    auto& jpoint = GetVertex(vertexInd);
    for (int i = 0; i < 3; i++) {
        auto v = jpoint[i].GetDouble();
        coordinates.push_back(v);
    }

    return coordinates.size() / 3 - 1;
}

//-----------------------------------------------------------------------------------------------
//
rapidjson::Value& Geometry::GetVertex(int vertexInd)
{
    if (m_bUseTemplateVerticies)
        return m_templateVerticies[vertexInd];
    else
        return m_jcityVerticies[vertexInd];
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
        THROW_ERROR("Geometry template index is out of range");
    }

    auto& tpl = m_templates[nTemplate];
    if (!tpl.json.IsNull()) {
        UseTemplateVerticies(true);
        tpl.item = ConvertItem(tpl.json);
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
    auto& jInsertPt = GetVertex(nInsertPt);
    for (int i = 0; i < 3; i++){
        double v = jInsertPt[i].GetDouble();
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

    const char* clsnames[] = { OWL_CityJsonPrefix  TYPE_GeometryInstance, OWL_Transformation, NULL};
    auto cls = m_cityModel.GetOrCreateClass(clsnames);
    
    GEOM::Transformation trans = CreateInstance(cls);
    trans.set_object(tpl.item);
    trans.set_matrix(T);

    return trans;
}
