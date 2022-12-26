#include "pch.h"
#include "CommonDefs.h"
#include "CityModel.h"
#include "cityJson2bin.h"
#include "Appearance.h"


//-----------------------------------------------------------------------------------------------
//
Appearance::Appearance(CityModel& cityModel) 
    : m_cityModel(cityModel) 
    , m_defaultThemeTexture (NULL)
    , m_defaultThemeMaterial (NULL)
{
}

//-----------------------------------------------------------------------------------------------
//
void Appearance::SetCityAppearance(rapidjson::Value& appearance)
{
    for (auto it = appearance.MemberBegin(); it != appearance.MemberEnd(); it++) {
        const char* name = it->name.GetString();
        
        if (!strcmp(name, MEMBER_MATERIALS)) {
            SetCityMaterials(it->value);
        }
        else if (!strcmp(name, MEMBER_TEXTURES)) {
            SetCityTextures(it->value);
        }
        else if (!strcmp(name, MEMBER_TXT_VERTICES)) {
            SetCityVerticiesTextures(it->value);
        }
        else if (!strcmp(name, MEMBER_DEF_TXT_THEME)) {
            m_defaultThemeTexture = it->value.GetString();
        }
        else if (!strcmp(name, MEMBER_DEF_MAT_THEME)) {
            m_defaultThemeMaterial = it->value.GetString();
        }
        else {
            TRACE_CNV("Unknown appearance member: %s\n", name);
        }
    }
}

static void GetDoubles(double d[], int n, rapidjson::Value& j)
{
    for (int i = 0; i < n; i++) {
        d[i] = j[i].GetDouble();
    }
}

//-----------------------------------------------------------------------------------------------
//
void Appearance::SetCityMaterials(rapidjson::Value& materials)
{
    for (auto& material : materials.GetArray()) {

        m_materials.push_back(Material());
        Material& m = m_materials.back();

        for (auto it = material.MemberBegin(); it != material.MemberEnd(); it++) {
            const char* memberName = it->name.GetString();

            if (!strcmp(memberName, MEMBER_NAME)) {
                m.name = it->value.GetString();
            }
            else if (!strcmp(memberName, MEMBER_AMBIENT_INTENSITY)) {
                m.ambientIntensity = it->value.GetDouble();
            }
            else if (!strcmp(memberName, MEMBER_DIFFUSE_COLOR)) {
                GetDoubles(m.diffuseColor, 3, it->value);
            }
            else if (!strcmp(memberName, MEMBER_EMMISSIVE_COLOR)) {
                GetDoubles(m.emissiveColor, 3, it->value);
            }
            else if (!strcmp(memberName, MEMBER_SPECULAR_COLOR)) {
                GetDoubles(m.specularColor, 3, it->value);
            }
            else if (!strcmp(memberName, MEMBER_SHININESS)) {
                m.shininess = it->value.GetDouble();
            }
            else if (!strcmp(memberName, MEMBER_TRANSPARENCY)) {
                m.transparency = it->value.GetDouble();
            }
            else if (!strcmp(memberName, MEMBER_SMOOTH)) {
                m.isSmooth = it->value.GetBool();
            }
            else {
                TRACE_CNV("Unknown material attribute: %s\n", memberName);
            }
        }
    }
}

//-----------------------------------------------------------------------------------------------
//
void Appearance::SetCityTextures(rapidjson::Value& textures)
{
    for (auto& texture : textures.GetArray()) {
        m_textures.push_back(Texture());
        Texture& t = m_textures.back();

        t.type = texture[MEMBER_TYPE].GetString();
        t.image = texture[MEMBER_IMAGE].GetString();
    }
}

//-----------------------------------------------------------------------------------------------
//
void Appearance::SetCityVerticiesTextures(rapidjson::Value& jverticies)
{
    assert(jverticies.IsArray());
    m_textureVerticies = jverticies;
    assert(jverticies.IsNull());
}

//-----------------------------------------------------------------------------------------------
//
GEOM::Color Appearance::GetDefaultColor()
{
    static GEOM::Color defaultColor;

    if (!defaultColor) {
        defaultColor = GEOM::Color::Create(m_cityModel.RdfModel(), "defaultColor");

        double clr[3] = { .5, .5, .5 };
        defaultColor.set_ambient(CreateColorComponent(clr));
        //??? rdfColor.set_ambientReflectance(m.ambientIntensity);

        defaultColor.set_diffuse(CreateColorComponent(clr)); //??? m.shininess
        defaultColor.set_emissive(CreateColorComponent(clr));
        defaultColor.set_specular(CreateColorComponent(clr));

        defaultColor.set_transparency(0.5);
    }

    return defaultColor;
}

//-----------------------------------------------------------------------------------------------
//
GEOM::Material Appearance::GetFaceMaterial(rapidjson::Value& jmaterial, rapidjson::Value& jtexture, IntList& faceIndexPath, rapidjson::Value& jrings)
{
    auto pmat = GetValue(jmaterial, m_defaultThemeMaterial, faceIndexPath);
    GEOM::Color color;
    if (pmat) {
        color = GetRdfColor(*pmat);
    }

    if (!color) {
        color = GetDefaultColor();
    }

    auto ptex = GetValue(jtexture, m_defaultThemeTexture, faceIndexPath);
    GEOM::Texture tex;
    //if (ptex)
    //    tex = GetRdfTexture(*ptex, jrings);

    if (color || tex) {
        auto mat = GEOM::Material::Create(m_cityModel.RdfModel());
        if (color)
            mat.set_color(color);
        if (tex)
            mat.set_textures(&tex, 1);
        return mat;
    }
    else {
        return NULL;
    }
}

//-----------------------------------------------------------------------------------------------
//
rapidjson::Value* Appearance::GetValue(rapidjson::Value& jnode, const char* defaultTheme, IntList& faceIndexPath)
{
    rapidjson::Value* theme = nullptr;

    if (!jnode.IsNull()) {
        if (defaultTheme) {
            auto it = jnode.FindMember(defaultTheme);
            if (it != jnode.MemberEnd()) {
                theme = &(it->value);
            }
        }

        if (!theme) {
            auto it = jnode.MemberBegin();
            if (it != jnode.MemberEnd()) {
                theme = &(it->value);
            }
        }
    }

    if (theme) {
        auto itvalues = theme->FindMember(MEMBER_VALUES);
        if (itvalues != theme->MemberEnd()) {
            auto values = &(itvalues->value);
            for (auto i : faceIndexPath) {
                values = &((*values)[i]);
            }
            return values;
        }
        else {
            auto itvalue = theme->FindMember(MEMBER_VALUE);
            if (itvalue != theme->MemberEnd()) {
                return &(itvalue->value);
            }
            else {
                LOG_CNV("Missed appearance value", "neither values nor value");
            }
        }
    }

    return nullptr;
}

//-----------------------------------------------------------------------------------------------
//
GEOM::ColorComponent Appearance::CreateColorComponent(double rgb[3], double w)
{
    auto clr = GEOM::ColorComponent::Create(m_cityModel.RdfModel());
    clr.set_R(rgb[0]);
    clr.set_G(rgb[1]);
    clr.set_B(rgb[2]);
    if (w >= 0) {
        clr.set_W(w);
    }
    return clr;
}

//-----------------------------------------------------------------------------------------------
//
GEOM::Color Appearance::GetRdfColor(rapidjson::Value& jmat)
{
    if (jmat.IsNull()) {
        return NULL;
    }

    try {
        auto indMat = jmat.GetInt();
        if (indMat >= m_materials.size()) {
            LOG_CNV("Material index out of range", "for surface material");
            return NULL;
        }

        auto& m = m_materials[indMat];

        if (!m.rdfColor) {
            
            auto rdfColor = GEOM::Color::Create(m_cityModel.RdfModel(), m.name);
            
            rdfColor.set_ambient(CreateColorComponent(m.diffuseColor));
            //??? rdfColor.set_ambientReflectance(m.ambientIntensity);
            
            rdfColor.set_diffuse(CreateColorComponent(m.diffuseColor)); //??? m.shininess
            rdfColor.set_emissive(CreateColorComponent(m.diffuseColor /*m.emissiveColor*/));
            rdfColor.set_specular(CreateColorComponent(m.diffuseColor  /*m.specularColor*/));

            rdfColor.set_transparency(1-m.transparency);

            m.rdfColor = rdfColor;
        }

        return m.rdfColor;
    }
    catch (cityJson2bin_error expt) {
        LOG_CNV("Failed to convert texture", expt.c_str());
    }

    return NULL;
}

//-----------------------------------------------------------------------------------------------
//
GEOM::Texture Appearance::GetRdfTexture(rapidjson::Value& jtex, rapidjson::Value& jrings)
{
    GEOM::Texture rdfTexture;

    try {
        auto& viOuter = jrings[0];  //outer loop vertex indecies
        auto& texOuter = jtex[0];  //texture for outer loop vertecies

        static Texture defaultTextute{ "JPG", "DefaultTexture.jpg" };
        Texture* pTexture = &defaultTextute;
        int texInd = texOuter[0].GetInt();
        if (texInd < m_textures.size()) {
            pTexture = &(m_textures[texInd]);
        }

        double uv[2][2] = { {0,0},{1,1} };
        double xyz[2][3] = { {0,0,0},{1,1,1} };
        for (int i = 0; i < 2; i++) {
            auto iptUV = texOuter[i + 1].GetInt();
            auto& jptUV = m_textureVerticies[iptUV];
            for (int k = 0; k < 2; k++)
                uv[i][k] = jptUV[k].GetDouble();

            auto iptXYZ = viOuter[i].GetInt();
            auto& jptXYZ = m_cityModel.GetVertex(iptXYZ);
            for (int k = 0; k < 3; k++)
                xyz[i][k] = jptXYZ[k].GetDouble();
        }

        rdfTexture = GEOM::Texture::Create(m_cityModel.RdfModel());
        rdfTexture.set_type(strcmp(pTexture->type, "JPG") ? 2 : 1);
        rdfTexture.set_name(pTexture->image);
        return 0;
        //TRACE_CNV("TODO textures....\n");

    }
    catch (cityJson2bin_error expt) {
        LOG_CNV ("Failed to convert texture", expt.c_str());
        rdfTexture = 0;
    }
    return rdfTexture;
}
