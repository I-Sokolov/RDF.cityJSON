#include "pch.h"
#include "CityModel.h"
#include "Settings.h"

#define CONFIG_FILE "CityJsonRDF.json"
#define SECTION_OPTIONS         "Options"
#define OPTION_USE_TEXTURES     "UseTextures"
#define OPTION_USE_MATERIALS    "UseMaterials"
#define SECTION_SEMANTIC_COLORS "SemanticColors"
#define SECTION_FILTERS         "Filters"

//-----------------------------------------------------------------------------------------------
//
Settings::Settings(CityModel& cityModel)
    :m_cityModel (cityModel)
{
    m_useMaterial = true;
    m_useTexture = true;
}

//-----------------------------------------------------------------------------------------------
//
void Settings::Load()
{
    auto path = GetSettingsFile();

    rapidjson::Document json;
    if (JsonReadFile(json, path.string().c_str())) {

        m_cityModel.LogMessage(ILog::Level::Info, "Load configuration file: %s", path.string().c_str());

        m_cityModel.State().PushMember("SettingFile");

        for (auto& section : json.GetObject()) {
            
            auto name = section.name.GetString();
            m_cityModel.State().PushMember(name);

            if (!strcmp(name, SECTION_OPTIONS)) {
                LoadOptions(section.value);
            }
            else if (!strcmp(name, SECTION_SEMANTIC_COLORS)) {
                LoadColors(section.value, m_semanticColors);
            }
            else if (!strcmp(name, SECTION_FILTERS)) {
                //LoadColors(section.value, m_semanticColors);
            }
            else {
                m_cityModel.LogMessage(ILog::Level::Warning, "Unknwon congiguration section '%s'", name);
            }

            m_cityModel.State().Pop();
        }
        m_cityModel.State().Pop();
    }
    else{
        m_cityModel.LogMessage(ILog::Level::Info, "Use default settings, configuration file not found: %s", path.string().c_str());
        return; //>>>>>>>>>>>>>>>>>
    }

}

//-----------------------------------------------------------------------------------------------
//
void Settings::LoadOptions(rapidjson::Value& opt)
{
    for (auto& o : opt.GetObject()) {

        auto name = o.name.GetString();
        m_cityModel.State().PushMember(name);

        if (!strcmp(name, OPTION_USE_MATERIALS)) {
            m_useMaterial = o.value.GetBool();
        }
        else if (!strcmp(name, OPTION_USE_TEXTURES)) {
            m_useTexture = o.value.GetBool();
        }
        else {
            m_cityModel.LogMessage(ILog::Level::Warning, "Unknwon congiguration option '%s'", name);
        }

        m_cityModel.State().Pop();
    }
}

//-----------------------------------------------------------------------------------------------
//
void Settings::LoadColors(rapidjson::Value& jcolors, NamedColors& colors)
{
    for (auto& jc : jcolors.GetObject()) {

        const char* name = jc.name.GetString();
        m_cityModel.State().PushMember(name);

        if (!jc.value.IsNull()) {
            auto ib = colors.insert(NamedColors::value_type(name, Color()));
            if (ib.second) {
                for (int i = 0; i<3; i++)
                ib.first->second.color[i] = jc.value[i].GetDouble();
            }
            else {
                m_cityModel.LogMessage(ILog::Level::Warning, "Color assigned more than once for '%s'", name);
            }
        }

        m_cityModel.State().Pop();
    }
}

//-----------------------------------------------------------------------------------------------
//
std::filesystem::path Settings::GetSettingsFile()
{
    char exePathBuff[1024] = "";
    GetModuleFileName(NULL, exePathBuff, _countof(exePathBuff));
    
    std::filesystem::path path(exePathBuff);
    path.replace_filename(CONFIG_FILE);

    return path;
}

//-----------------------------------------------------------------------------------------------
//
GEOM::Color Settings::GetSemanticColor(OwlInstance semantic)
{
    auto model = GetModel(semantic);
    auto semanticName = GetNameOfInstance(semantic);
    if (!semanticName)
        return NULL;

    auto it = m_semanticColors.find(semanticName);
    if (it == m_semanticColors.end())
        return NULL;

    auto& clr = it->second;
    if (clr.rdfColor == NULL) {
        clr.rdfColor = GEOM::Color::Create(model);

        auto c = GEOM::ColorComponent::Create(model);
        c.set_R(clr.color[0]);
        c.set_G(clr.color[1]);
        c.set_B(clr.color[2]);
        c.set_W(1);

        clr.rdfColor.set_ambient(c);
        clr.rdfColor.set_diffuse(c);
        clr.rdfColor.set_emissive(c);
        clr.rdfColor.set_specular(c);
        clr.rdfColor.set_ambientReflectance(1);
    }

    return clr.rdfColor;
}
