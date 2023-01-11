#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string>
#include <set>

#include "engine.h"
#include "cityJson2bin.h"

//-----------------------------------------------------------------------------------------------
//
static void SaveBinFile(OwlModel model, const char* rdfFilePath)
{
    auto fp = fopen(rdfFilePath, "w");
    if (!fp) {
        printf("ERROR: Filed to open output file\n");
        return;
    }
    fclose(fp);

    auto res = SaveModel(model, rdfFilePath);
    if (res)
        printf("ERROR: Failed to write output file\n");
}

//-----------------------------------------------------------------------------------------------
//
struct Progress : public cityJson2bin::IProgress
{
    virtual void Start(int range) override
    {
        printf("PROGRESS: Start creating city objects...\n");
        m_range = range;
    }

    virtual void Step() override
    {
        m_step++;
        int p = (int)(10 * m_step / m_range);
        if (p > m_proc) {
            m_proc = p;
            printf("PROGRESS: %d%%...\n", m_proc * 10);
        }
    }

    virtual void Finish() override
    {
        printf("PROGRESS: Finish reading\n");
    }

private:
    int m_range = 1;
    int m_step = 0;
    int m_proc = 0;
};

//-----------------------------------------------------------------------------------------------
//
struct Log : public cityJson2bin::ILog
{
    virtual void Message(Level level, const char* category, const char* msg, const char* converterState) override
    {
        if (!m_caregorites.insert(category).second) {
            return; //show each category once
        }

        const char* lev = "UNKNOWN LEVER";
        switch (level) {
            case cityJson2bin::ILog::Level::Error: lev = "ERROR"; break;
            case cityJson2bin::ILog::Level::Warning: lev = "WARING"; break;
            case cityJson2bin::ILog::Level::Info: lev = "INFO"; break;
        }

        printf("++ LOG %s: %s (at %s)\n", lev, msg, converterState);
    }

    std::set<std::string> m_caregorites;
};

//-----------------------------------------------------------------------------------------------
//
int main(int argc, const char* argv[])
{
    if (argc < 3) {
        printf("USAGE:\n");
        printf("\t%s <input cityJSON file path> <output RDF bin file path>\n", argv[0]);
        return -1;
    }

    printf("Converting %s to %s....\n", argv[1], argv[2]);

    Progress progress;
    Log log;

    auto model = cityJson2bin::Open(argv[1], &progress, &log);
    if (model) {
        SaveBinFile(model, argv[2]);
    }

    return 0;
}


