//==== MapBuilder -> Written by Unusuario2, https://github.com/Unusuario2  ====//
//
// Purpose:
//
// $NoKeywords: $
//==============================================================================//
#ifndef MAPBUILDER_HPP
#define MAPBUILDER_HPP

#ifdef _WIN32
#pragma once
#endif // _WIN32


//-----------------------------------------------------------------------------
// Purpose: defs of KeyValues
//-----------------------------------------------------------------------------
#define KV_MAPBUILDERSYSTEM     "MapBuilderSystem"
#define KV_DEFAULTBUILDSETTINGS "DefaultMapBuilderSettings"
#define KV_COMPILEPRESET        "CompilePresets"
#define KV_WORLDBUILDER         "RunWorldBuilder"
#define KV_VISIBILITYBUILDER    "RunVisibilityBuilder"
#define KV_LIGHTBUILDER         "RunLightBuilder"
#define KV_CUBEMAPBUILDER       "RunCubemapBuilder"
#define KV_COPY_FILE            "RunTrasferFile"
#define KV_MAPINFOBUILDER       "RunMapInfo"
#define KV_CUSTOMBUILDER        "RunCustomBuilder"
#define KV_BINDIR               "BinDir"
#define KV_BASEDIR              "BaseDir"
#define KV_EXTERNALPATH         "ExternalPath"
#define KV_TOOLNAME             "ToolName"
#define KV_BUILDPARAMS          "BuildParams"


#define FILE_MAPBUILDER_SETTINGS "scripts/tools/mapbuilder_settings.txt"


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
struct Builder_t
{
public:
    bool m_bRunPreset = false;
    char m_szToolName[MAX_PATH];
    char m_szToolBasePath[MAX_PATH];
    char m_szFullCommandLine[MAX_CMD_BUFFER_SIZE];

public:
    Builder_t::Builder_t(const bool bRunBuilder, const char* pToolName, const char* pToolBaseDir, const char* pBuildArgv)
    {
        m_bRunPreset = bRunBuilder;
        V_strcpy_safe(m_szToolName, pToolName);
        V_strcpy_safe(m_szToolBasePath, pToolBaseDir);

        V_sprintf_safe(m_szFullCommandLine, "\"%s\\%s\" %s", m_szToolBasePath, m_szToolName, pBuildArgv);
    }

    Builder_t::~Builder_t() {}
};


//-----------------------------------------------------------------------------
// Purpose: Every tool has a indivudal keyvalue associated.
//-----------------------------------------------------------------------------
struct PairKvTl
{
public:
    char m_szKeyValue[256];
    char m_szToolName[MAX_PATH];
    char m_szBaseDir[MAX_PATH];

public:
    PairKvTl::PairKvTl(const char* pKeyValue, const char* pToolName, const char* pBaseToolDir)
    {
        V_strcpy_safe(m_szKeyValue, pKeyValue);
        V_strcpy_safe(m_szToolName, pToolName);
        V_strcpy_safe(m_szBaseDir, pBaseToolDir);
    }

    PairKvTl::~PairKvTl() {}
};

#endif // MAPBUILDER_HPP

