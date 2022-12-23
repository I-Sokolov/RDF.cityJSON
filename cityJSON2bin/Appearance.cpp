#include "pch.h"
#include "CommonDefs.h"
#include "CityModel.h"
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
            TRACE("Unknown appearance member: %s\n", name);
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
        const char* materialName = nullptr;
        Material m;

        for (auto it = material.MemberBegin(); it != material.MemberEnd(); it++) {
            const char* memberName = it->name.GetString();
            
            if (!strcmp(memberName, MEMBER_NAME)) {
                materialName = it->value.GetString();
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
                TRACE("Unknown transparency attribute: %s\n", memberName);
            }
        }

        if (materialName) {
            m_materials.insert(Materials::value_type(materialName, m));
        }
        else {
            THROW_ERROR("Missed material name");
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
GEOM::Material Appearance::GetFaceMaterial(rapidjson::Value& jmaterial, rapidjson::Value& jtexture, int iface)
{
    auto pmat = GetValue(jmaterial, m_defaultThemeMaterial, iface);
    GEOM::Color color;
    if (pmat)
        color = GetRdfColor(*pmat);

    auto ptex = GetValue(jtexture, m_defaultThemeTexture, iface);
    GEOM::Texture tex;
    if (ptex)
        tex = GetRdfTexture(*ptex);

    if (color || tex) {
        auto mat = GEOM::Material::Create(m_cityModel.RdfModel());
        mat.set_color(color);
        mat.set_textures(&tex, 1);
        return mat;
    }
    else {
        return NULL;
    }
}

//-----------------------------------------------------------------------------------------------
//
rapidjson::Value* Appearance::GetValue(rapidjson::Value& jnode, const char* defaultTheme, int index)
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
        theme = &((*theme)[MEMBER_VALUES][index]);
    }

    return theme;
}

//-----------------------------------------------------------------------------------------------
//
GEOM::Color Appearance::GetRdfColor(rapidjson::Value& jmat)
{
    return 0;
}

//-----------------------------------------------------------------------------------------------
//
GEOM::Texture Appearance::GetRdfTexture(rapidjson::Value& jtex)
{
    if (jtex.IsArray()) {
        auto& pt1Texture = jtex[0];
        int texInd = pt1Texture[0].GetInt();
        if (texInd < m_textures.size()) {

        }
    }
    return 0;
}
