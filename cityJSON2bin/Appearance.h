#pragma once

class CityModel;

class Appearance
{
public:
    Appearance(CityModel& cityModel);

    void SetCityAppearance(rapidjson::Value& appearance);

private:
    void SetCityMaterials(rapidjson::Value& materials);
    void SetCityTextures(rapidjson::Value& textures);
    void SetCityVerticiesTextures(rapidjson::Value& verticies);

private:
    struct Material
    {
        double ambientIntensity = 0;
        double diffuseColor[3]  = {0,0,0};
        double emissiveColor[3] = { 0,0,0 };
        double specularColor[3] = { 0,0,0 };
        double shininess        = 0;
        double transparency     = 0;
        bool   isSmooth         = false;
    };

    typedef std::map<std::string, Material> Materials;

    struct Texture
    {
        const char* type = nullptr;
        const char* image = nullptr;
    };

    typedef std::vector<Texture> Textures;

private:
    CityModel&           m_cityModel;

    Materials            m_materials;
    Textures             m_textures;
    rapidjson::Value     m_textureVerticies;
    const char*          m_defaultThemeTexture;
    const char*          m_defaultThemeMaterial;
};

