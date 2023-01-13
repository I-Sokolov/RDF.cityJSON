#pragma once

class CityModel;

class Settings
{
public:
    Settings(CityModel& model);

    void Load();

    bool UseMaterial() { return m_useMaterial; }
    bool UseTexture() { return m_useTexture; }
    
    GEOM::Color GetSemanticColor(OwlInstance semantic);

private:
    struct Color
    {
        double           color[3];
        GEOM::Color      rdfColor = 0;
    };

    typedef std::map<std::string, Color> NamedColors;

private:
    std::filesystem::path GetSettingsFile();
    void LoadOptions(rapidjson::Value& opt);
    void LoadColors(rapidjson::Value& jcolors, NamedColors& colors);

private:
    CityModel& m_cityModel;

    bool m_useMaterial;
    bool m_useTexture;

    NamedColors  m_semanticColors;
 
};

