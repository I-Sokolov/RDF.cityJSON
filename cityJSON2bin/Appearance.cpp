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
void Appearance::GetSurfaceAppearance(SurfaceAppearance& appearance, rapidjson::Value& jmaterial, rapidjson::Value& jtexture, UIntList& faceIndexPath)
{
    if (jmaterial.IsObject()) {
        for (auto& jmat : jmaterial.GetObject()) {
            const char* theme = jmat.name.GetString();
            auto pmat = FindValueByIndexPath(jmat.value, faceIndexPath);
            if (pmat && pmat->IsInt()) {
                auto ind = pmat->GetInt();
                appearance.materials.insert(Theme2Index::value_type(theme, ind));
            }
        }
    }

    if (jtexture.IsObject()) {
        for (auto& jtex : jtexture.GetObject()) {
            const char* theme = jtex.name.GetString();

            auto ptex = FindValueByIndexPath(jtex.value, faceIndexPath);
            if (ptex && ptex->IsArray()) {

                bool hasNull = false;
                int texInd = -1;
                ListOfListOfInt uv2;

                for (auto& ri : ptex->GetArray()) {
                    if (hasNull) {
                        break;
                    }

                    uv2.push_back(ListOfInt());
                    auto& uv = uv2.back();

                    if (ri.IsArray()) {
                        for (rapidjson::SizeType i = 0; i < ri.Size(); i++) {
                            if (ri[i].IsNull()) {
                                hasNull = true;
                                break;
                            }
                            else if (i == 0) {
                                if (texInd < 0) {
                                    texInd = ri[i].GetInt();
                                }
                                else if (texInd != ri[i].GetInt()) {
                                    LOG_CNV("texture for opening is differ from texture for outer", "");
                                }
                            }
                            else {
                                uv.push_back(ri[i].GetInt());
                            }
                        }
                    }
                }

                if (!hasNull) {
                    auto ib1 = appearance.textures.insert(Theme2Index::value_type(theme, texInd));
                    auto ib2 = appearance.textureVerticies.insert(Theme2TextureIndecies::value_type(theme, ListOfListOfInt()));
                    std::swap(ib2.first->second, uv2);
                }
            }
        }
    }
}

//-----------------------------------------------------------------------------------------------
//
int Appearance::GetThemeIndex(Theme2Index& th2ind, const char* defaultTheme, size_t maxInd)
{
    int ind = -1;
    if (defaultTheme) {
        auto it = th2ind.find(defaultTheme);
        if (it != th2ind.end()) {
            ind = it->second;
        }
    }
    else if (maxInd > 0) {
        ind = 0;
    }

    if (ind > 0 && ind >= maxInd) {
        LOG_CNV("Material or texture index is out of range", "");
        ind = -1;
    }

    return ind;
}

//-----------------------------------------------------------------------------------------------
//
GEOM::Material Appearance::GetRdfMaterial(Theme2Index& materials, Theme2Index& textures)
{
    int iMat = GetThemeIndex(materials, m_defaultThemeMaterial, m_materials.size());
    int iTex = GetThemeIndex(textures, m_defaultThemeTexture, m_textures.size());

    auto& tex2rdf = m_matTex2Rdf[iMat];
    auto& rdfMat = tex2rdf[iTex];

    if (!rdfMat) {
        GEOM::Color color = GetRdfColor(iMat);
        GEOM::Texture tex = GetRdfTexture(iTex);

        rdfMat = GEOM::Material::Create(m_cityModel.RdfModel());
        if (color)
            rdfMat.set_color(color);
        if (tex)
            rdfMat.set_textures(&tex, 1);
    }

    return rdfMat;
}

//-----------------------------------------------------------------------------------------------
//
rapidjson::Value* Appearance::FindValueByIndexPath(rapidjson::Value& jnode, UIntList& faceIndexPath)
{
    auto itvalue = jnode.FindMember(MEMBER_VALUE);
    if (itvalue != jnode.MemberEnd()) {
        return &(itvalue->value);
    }

    auto itvalues = jnode.FindMember(MEMBER_VALUES);
    if (itvalues != jnode.MemberEnd()) {
        auto values = &(itvalues->value);
        for (auto i : faceIndexPath) {
            if (values) {
                values = &((*values)[i]);
            }
        }
        return values;
    }

    LOG_CNV("Missed appearance value", "neither values nor value");
    return nullptr;
}

//-----------------------------------------------------------------------------------------------
//
GEOM::ColorComponent Appearance::CreateColorComponent(double rgb[3], double scale)
{
    if (scale < 0 || scale > 1)
        scale = 1;

    auto clr = GEOM::ColorComponent::Create(m_cityModel.RdfModel());
    clr.set_R(rgb[0]*scale);
    clr.set_G(rgb[1]*scale);
    clr.set_B(rgb[2]*scale);

    return clr;
}

//-----------------------------------------------------------------------------------------------
//
GEOM::Color Appearance::GetRdfColor(int iMat)
{
    if (iMat >= 0 && iMat < m_materials.size()) {
        auto& m = m_materials[iMat];

        if (!m.rdfColor) {

            auto rdfColor = GEOM::Color::Create(m_cityModel.RdfModel(), m.name);

            rdfColor.set_ambient(CreateColorComponent(m.diffuseColor, m.ambientIntensity));

            rdfColor.set_diffuse(CreateColorComponent(m.diffuseColor)); 
            rdfColor.set_emissive(CreateColorComponent(m.emissiveColor));
            rdfColor.set_specular(CreateColorComponent(m.specularColor));
            
            //??? m.shininess
            rdfColor.set_transparency(1 - m.transparency);

            m.rdfColor = rdfColor;
        }

        return m.rdfColor;
    }
    else {
        return NULL;
    }
}

//-----------------------------------------------------------------------------------------------
//
GEOM::Texture Appearance::GetRdfTexture(int iTex)
{
    if (iTex >= 0 && iTex < m_textures.size()) {
        auto& t = m_textures[iTex];

        if (!t.rdfTexture) {
            t.rdfTexture = GEOM::Texture::Create(m_cityModel.RdfModel());
            t.rdfTexture.set_type(strcmp(t.type, "JPG") ? 2 : 1);
            t.rdfTexture.set_name(t.image);

        }
        return t.rdfTexture;
    }
    else {
        return NULL;
    }
}
