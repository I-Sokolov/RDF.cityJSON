#pragma once

class CityModel;

class Appearance
{
public:
    typedef std::map<std::string, int> Theme2Index;
    typedef std::map<std::string, ListOfListOfInt> Theme2TextureIndecies;

    struct SurfaceAppearance
    {
        Theme2Index             materials;
        Theme2Index             textures;
        Theme2TextureIndecies   textureIndecies;
    };

public:
    Appearance(CityModel& cityModel);

public:
    void SetCityAppearance(rapidjson::Value& appearance);

    void GetSurfaceAppearance(SurfaceAppearance& appearance, rapidjson::Value& jmaterial, rapidjson::Value& jtexture, UIntList& faceIndexPath);
    
    GEOM::Material GetRdfMaterial(Theme2Index& materials, Theme2Index& textures, OwlInstance semantic);
    ListOfListOfInt* GetTextuteIndecies(Theme2Index& textures, Theme2TextureIndecies& textureIndecies);
    rapidjson::Value& GetTextureVertex(int ind) { return m_textureVerticies[ind]; }

private:
    void SetCityMaterials(rapidjson::Value& materials);
    void SetCityTextures(rapidjson::Value& textures);
    void SetCityVerticiesTextures(rapidjson::Value& verticies);

private:
    struct Material
    {
        const char* name        = nullptr;
        double ambientIntensity = 0;
        double diffuseColor[3]  = {0,0,0};
        double emissiveColor[3] = { 0,0,0 };
        double specularColor[3] = { 0,0,0 };
        double shininess        = 0;
        double transparency     = 0;
        bool   isSmooth         = false;

        GEOM::Color rdfColor    = NULL;
    };

    typedef std::vector<Material>         Materials;

    struct Texture
    {
        const char* type = nullptr;
        const char* image = nullptr;

        GEOM::Texture rdfTexture = NULL;
    };

    typedef std::vector<Texture> Textures;

    typedef std::map<OwlInstance, GEOM::Material> Semantic2Rdf;
    typedef std::map<int, Semantic2Rdf> Tex2Rdf;
    typedef std::map<int, Tex2Rdf> MatTex2Rdf;

private:
    rapidjson::Value* FindValueByIndexPath(rapidjson::Value& jnode, UIntList& faceIndexPath);    

    const char* GetActiveTheme(Theme2Index& th2ind, const char* defaultTheme);
    int GetThemeIndex(Theme2Index& th2ind, const char* defaultTheme, size_t maxInd);

    GEOM::Color GetRdfColor(int iMat, OwlInstance semantic);
    GEOM::ColorComponent CreateColorComponent(double rgb[3], double scale = -1);
    GEOM::Texture GetRdfTexture(int iTex);

private:
    CityModel&           m_cityModel;

    Materials            m_materials;
    Textures             m_textures;
    rapidjson::Value     m_textureVerticies;
    const char*          m_defaultThemeTexture;
    const char*          m_defaultThemeMaterial;
    MatTex2Rdf           m_matTex2Rdf;
};

