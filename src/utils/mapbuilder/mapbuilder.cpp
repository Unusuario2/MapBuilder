//===== MapBuilder -> Written by Unusuario2, https://github.com/Unusuario2  ====//
//
// Purpose: 
//
// License:
//        MIT License
//
//        Copyright (c) 2025 [un usuario], https://github.com/Unusuario2
//
//        Permission is hereby granted, free of charge, to any person obtaining a copy
//        of this software and associated documentation files (the "Software"), to deal
//        in the Software without restriction, including without limitation the rights
//        to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//        copies of the Software, and to permit persons to whom the Software is
//        furnished to do so, subject to the following conditions:
//
//        The above copyright notice and this permission notice shall be included in all
//        copies or substantial portions of the Software.
//
//        THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//        IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//        FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//        AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//        LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//        OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//        SOFTWARE.
// $NoKeywords: $
//==============================================================================//
#include <windows.h>
#include <vector>
#include <tier1/strtools.h>
#include <tier0/icommandline.h>
#include <tools_minidump.h>
#include <loadcmdline.h>
#include <cmdlib.h>
#include <KeyValues.h>
#include <filesystem_init.h>
#include <filesystem_tools.h>
#include <colorschemetools.h>
#include <pipeline_shareddefs.h>
#include <resourcecopy/cresourcecopy.hpp>
#include "mapbuilder.hpp"


#pragma warning(disable:4238)

// Unusuario2: at the moment we call vbsp, vvis, vrad, if we move (as we should) to a system like source 2 were we have resourcecompile.exe then set this to 1.
#define RESOURCECOMPILER_SYSTEM 0


//-----------------------------------------------------------------------------
// Purpose: Global vars 
//-----------------------------------------------------------------------------
char            g_szBaseToolDir[MAX_PATH];
char            g_szBaseGlobalDir[MAX_PATH];
char            g_szSourceFile[MAX_PATH];
bool            g_bDefaultPresetUser = false;
bool            g_bIgnoreErrors = false;
#if RESOURCECOMPILER_SYSTEM
bool            g_bRunWorld = false;
bool            g_bRunVis = false;
bool            g_bRunVrad = false;
bool            g_bRunMapInfo = false;
bool            g_bRunCubemap = false;
bool            g_bRunNav = false;
bool            g_bRunAin = false;
#endif // RESOURCECOMPILER_SYSTEM
char            g_szPresetName[MAX_PATH] = { '\0' };
SpewMode        g_eSpewMode = SpewMode::k_Normal;
CResourceCopy*  g_pResourceCopy = nullptr;


//-----------------------------------------------------------------------------
// Purpose:     
// Supported:
// - %filename : (e.g: test)
// - %map : (e.g: C:\test\testmap)
// - %gamedir: Path to gameinfo.txt
// - %mapdir: gamedir/maps
//-----------------------------------------------------------------------------
const char* ParseStringBuildParams(const char* pBuildParams)
{
    static char s_RetString[MAX_CMD_BUFFER_SIZE];
    std::vector<char> Temp;
    Temp.reserve(MAX_CMD_BUFFER_SIZE);

    int iLen = V_strlen(pBuildParams);
    for (int i = 0; i < iLen; )
    {
        // Look for a '%' placeholder
        if (pBuildParams[i] == '%')
        {
            if (!V_strnicmp(&pBuildParams[i], "%filename", 9))
            {
                char szFileName[MAX_PATH];
                V_strcpy_safe(szFileName, (V_strrchr(g_szSourceFile, '\\') ? V_strrchr(g_szSourceFile, '\\') + 1 : g_szSourceFile));
                char* pDot = V_strrchr(szFileName, '.');
                if (pDot) 
                    *pDot = '\0';
                
                {
                    int len = V_strlen(szFileName);
                    for (int j = 0; j < len; ++j)
                        Temp.push_back(szFileName[j]);
                }
                i += 9;
            }
            else if (!V_strnicmp(&pBuildParams[i], "%mapdir", 7))
            {
                char szTemp[MAX_PATH];
                V_sprintf_safe(szTemp, "%s\\%s", gamedir, DIR_MAPS);

                Temp.push_back('"');
                {
                    int len = V_strlen(szTemp);
                    for (int j = 0; j < len; ++j)
                        Temp.push_back(szTemp[j]);
                }
                Temp.push_back('"');

                i += 7;
            }
            else if (!V_strnicmp(&pBuildParams[i], "%source", 7))
            {
                Temp.push_back('"');
                {
                    int len = V_strlen(g_szSourceFile);
                    for (int j = 0; j < len; ++j)
                        Temp.push_back(g_szSourceFile[j]);
                }
                Temp.push_back('"');
                i += 7;
            }
            else if (!V_strnicmp(&pBuildParams[i], "%mapbsp", 7))
            {
                char szTemp[MAX_PATH];
                V_strcpy_safe(szTemp, g_szSourceFile);
                char* pDot = V_strrchr(szTemp, '.');
                if (pDot)
                    *pDot = '\0';

                V_sprintf_safe(szTemp, "%s.bsp", szTemp);
                Temp.push_back('"');
                {
                    int len = V_strlen(szTemp);
                    for (int j = 0; j < len; ++j)
                        Temp.push_back(szTemp[j]);
                }
                Temp.push_back('"');
                i += 7;
            }
            else if (!V_strnicmp(&pBuildParams[i], "%gamedir", 8))
            {
                Temp.push_back('"');
                {
                    int len = V_strlen(gamedir);
                    for (int j = 0; j < len; ++j)
                        Temp.push_back(gamedir[j]);
                    Temp.push_back('"');
                }
                i += 8;
            }
            else
            {
                Warning("MapBuilder -> Unknown parameter near: '%s'. Only %%filename, %%source, %%mapbsp, %%gamedir are supported.\n", &pBuildParams[i]);
            }
        }

        // Default: just copy character
        Temp.push_back(pBuildParams[i]);
        ++i;
    }

    Temp.push_back('\0');
    V_strcpy_safe(s_RetString, Temp.data());
    return s_RetString;
}


//-----------------------------------------------------------------------------
// Purpose:     
//-----------------------------------------------------------------------------
static const std::vector<Builder_t> ParseMapBuilderScript()
{
    float start = Plat_FloatTime();
    if (g_eSpewMode >= SpewMode::k_Verbose) { Msg("MapBuilder -> Parsing script from: %s", FILE_MAPBUILDER_SETTINGS); }

    KeyValues* pKvMapBuilderSettings = new KeyValues("");
    if (!pKvMapBuilderSettings->LoadFromFile(g_pFullFileSystem, FILE_MAPBUILDER_SETTINGS))
            Error("\nMapBuilder -> Could not load KeyValues from %s", FILE_MAPBUILDER_SETTINGS);

    // if the user does not set the preset set the default one
    if (!g_bDefaultPresetUser)
    {
        const char* pPreset = pKvMapBuilderSettings->GetString(KV_DEFAULTBUILDSETTINGS, nullptr);
        if (!pPreset)
            Error("\nMapBuilder -> Could not get the default KeyValue '%s' preset!\n", KV_DEFAULTBUILDSETTINGS);
        V_strcpy_safe(g_szPresetName, pPreset);
    }

    KeyValues* pKvCompilePresets = pKvMapBuilderSettings->FindKey(KV_COMPILEPRESET);
    if (!pKvCompilePresets)
        Error("\nMapBuilder -> Could not get the '%s' KeyValue!\n", KV_COMPILEPRESET);

    KeyValues* pKvPreset = pKvCompilePresets->FindKey(g_szPresetName);
    if (!pKvPreset)
        Error("\nMapBuilder -> Preset '%s' is does not exist in %s!\n", g_szPresetName, FILE_MAPBUILDER_SETTINGS);

    // Now we scan inside the preset all the proccess to be exec
    std::vector<Builder_t> RetList;
    {
        PairKvTl BaseKvList[] = {   PairKvTl(KV_WORLDBUILDER, TOOL_MAP_VBSP, g_szBaseToolDir), PairKvTl(KV_VISIBILITYBUILDER, TOOL_MAP_VVIS, g_szBaseToolDir),
                                    PairKvTl(KV_LIGHTBUILDER, TOOL_MAP_VRAD, g_szBaseToolDir), PairKvTl(KV_COPY_FILE, TOOL_COPY, g_szBaseToolDir),
                                    PairKvTl(KV_MAPINFOBUILDER, TOOL_MAP_INFO, g_szBaseToolDir), PairKvTl(KV_CUBEMAPBUILDER, " " ,  g_szBaseGlobalDir)};

        // Scann all of the keyvalues inside the preset
        for (KeyValues* Kv = pKvPreset->GetFirstSubKey(); Kv; Kv = Kv->GetNextKey())
        {
            // Sanity check!
            const char* pName = Kv->GetName();
            if (!pName)
                Error("\nMapBuilder -> Could not get the name of the Key Child inside '%s' KeyValue.\n"
                    "MapBuilder -> Write a Child KV!\n", KV_COMPILEPRESET);
            
            // We allow users to order their normal of custom builders, for that we need to know if the KeyName is the same as the KeyValue and parse it.
            if ([&BaseKvList, &pName]() -> const bool
                {
                    return  !V_stricmp(pName, BaseKvList[0].m_szKeyValue) || 
                            !V_stricmp(pName, BaseKvList[1].m_szKeyValue) ||
                            !V_stricmp(pName, BaseKvList[2].m_szKeyValue) ||
                            !V_stricmp(pName, BaseKvList[3].m_szKeyValue) ||
                            !V_stricmp(pName, BaseKvList[4].m_szKeyValue) ||
                            !V_stricmp(pName, BaseKvList[5].m_szKeyValue);
                }()
                    )
            {
                const char* pToolName = Kv->GetString(KV_TOOLNAME, nullptr);
                const char* pBuildParams = Kv->GetString(KV_BUILDPARAMS, nullptr);
                if (!pBuildParams)
                    Error("MapBuilder -> Missing KeyValue: '%s' of '%s'!\n", KV_BUILDPARAMS, pName);

                // Make the full commandline
                Builder_t Obj = Builder_t(true, pToolName ? pToolName : [&BaseKvList, &pName]() -> const char*
                                                                        {
                                                                            if (!V_stricmp(pName, BaseKvList[0].m_szKeyValue))
                                                                                return BaseKvList[0].m_szToolName;
                                                                            else if (!V_stricmp(pName, BaseKvList[1].m_szKeyValue))
                                                                                return BaseKvList[1].m_szToolName;
                                                                            else if (!V_stricmp(pName, BaseKvList[2].m_szKeyValue))
                                                                                return BaseKvList[2].m_szToolName;
                                                                            else if (!V_stricmp(pName, BaseKvList[3].m_szKeyValue))
                                                                                return BaseKvList[3].m_szToolName;
                                                                            else if (!V_stricmp(pName, BaseKvList[4].m_szKeyValue))
                                                                                return BaseKvList[4].m_szToolName;
                                                                            else if (!V_stricmp(pName, BaseKvList[5].m_szKeyValue))
                                                                                return BaseKvList[5].m_szToolName;
                                                                        }()
                    
                                                                        ,[&BaseKvList, &pName]() -> const char*
                                                                            {
                                                                                if (!V_stricmp(pName, BaseKvList[0].m_szKeyValue))
                                                                                    return BaseKvList[0].m_szBaseDir;
                                                                                else if (!V_stricmp(pName, BaseKvList[1].m_szKeyValue))
                                                                                    return BaseKvList[1].m_szBaseDir;
                                                                                else if (!V_stricmp(pName, BaseKvList[2].m_szKeyValue))
                                                                                    return BaseKvList[2].m_szBaseDir;
                                                                                else if (!V_stricmp(pName, BaseKvList[3].m_szKeyValue))
                                                                                    return BaseKvList[3].m_szBaseDir;
                                                                                else if (!V_stricmp(pName, BaseKvList[4].m_szKeyValue))
                                                                                    return BaseKvList[4].m_szBaseDir;
                                                                                else if (!V_stricmp(pName, BaseKvList[5].m_szKeyValue))
                                                                                    return BaseKvList[5].m_szBaseDir;
                                                                            }()
                                                                        , ParseStringBuildParams(pBuildParams));
                // Check first if the tool exists...
                if (Obj.m_bRunPreset)
                {
                    char szFullToolPath[MAX_PATH];
                    V_sprintf_safe(szFullToolPath, "%s\\%s", Obj.m_szToolBasePath, Obj.m_szToolName);
                    if (!g_pResourceCopy->FileExist(szFullToolPath))
                        Error("\nMapBuilder -> Tool: %s does not exist!\n", szFullToolPath);
                }
                RetList.push_back(Obj);
            }
            else if (!V_stricmp(pName, KV_CUSTOMBUILDER))
            {
                // For custom builder we allow more custom things so the user can set custom stuff.
                char szBasePath[MAX_PATH] = { '\0' };
                char szToolName[MAX_PATH] = { '\0' };

                if (Kv->GetBool(KV_BINDIR))
                    V_strcpy_safe(szBasePath, g_szBaseToolDir);
                else if (Kv->GetBool(KV_BASEDIR))
                    V_strcpy_safe(szBasePath, g_szBaseGlobalDir);
                else if (Kv->GetString(KV_EXTERNALPATH, nullptr))
                    V_strcpy_safe(szBasePath, Kv->GetString(KV_EXTERNALPATH, nullptr));
                else if (Kv->GetString(KV_TOOLNAME, nullptr))
                    V_strcpy_safe(szToolName, Kv->GetString(KV_TOOLNAME, nullptr));
                else if (Kv->GetString(KV_BUILDPARAMS, nullptr))
                    RetList.push_back(Builder_t(true, szToolName, szBasePath, ParseStringBuildParams(Kv->GetString(KV_BUILDPARAMS, nullptr))));
                else
                    Error("MapBuilder -> Unknow parameter\n");

                // Check if the tools exists...
                {
                    char szToolDir[MAX_PATH];
                    V_sprintf_safe(szToolDir, "%s\\%s", szBasePath, szToolName);
                    if (!g_pResourceCopy->FileExist(szToolDir))
                        Error("\nMapBuilder -> (%s) Tool: %s does not exist!\n", KV_CUSTOMBUILDER, szToolDir);
                }
            }
        }
    }
    
    pKvMapBuilderSettings->deleteThis();

    if (g_eSpewMode >= SpewMode::k_Verbose) { Msg("done(%.2f)\n", Plat_FloatTime() - start); }

    return RetList;
}


//----------------------------------------------------------------------------
// Purpose: Starts .exe tools
//----------------------------------------------------------------------------
static void StartExe(const char* pFullCommand)
{
    float start = Plat_FloatTime();

    Msg("\nAssetTools -> Starting: "); ColorSpewMessage(SPEW_MESSAGE, &ColorPath, "%s\n\n", pFullCommand);

    STARTUPINFOA si = { 0 };
    PROCESS_INFORMATION pi = { 0 };
    si.cb = sizeof(si);

    DWORD dwFlags = 0;

    {
        // This is so fucking retarded...
        char* szTemp = V_strdup(pFullCommand);
        if (!CreateProcessA(NULL, szTemp, NULL, NULL, TRUE, dwFlags, NULL, NULL, &si, &pi))
        {
            Error("AssetTools -> %s could not start!\n", pFullCommand);
            return;
        }
        delete[] szTemp;
    }
    // Wait until child process exits
    WaitForSingleObject(pi.hProcess, INFINITE);

    // Retrieve exit code
    DWORD exitCode = 0;
    if (GetExitCodeProcess(pi.hProcess, &exitCode))
    {
        if ((exitCode != 0) && (!g_bIgnoreErrors))
        {
            Error("AssetTools -> %s compile failed: %d!\n", pFullCommand, exitCode);
        }
    }
    else
    {
        Warning("AssetTools -> GetExitCodeProcess() failed!\n");
    }

    // Close process handles
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    ColorSpewMessage(SPEW_MESSAGE, &ColorSucesfull, "AssetTools -> Done building in %.2f seconds.\n", Plat_FloatTime() - start);
}


//-----------------------------------------------------------------------------
// Purpose:     
//-----------------------------------------------------------------------------
static void RunMapBuilder()
{
    float start = Plat_FloatTime();

    // Check if the source map exists
    if(!g_pResourceCopy->FileExist(g_szSourceFile))
        Error("MapBuilder -> Map file: %s does not exist!\n", g_szSourceFile);

    const std::vector<Builder_t> BuilderList = ParseMapBuilderScript();

    // Execute the command
    for (auto& Builder : BuilderList)
    {
        StartExe(Builder.m_szFullCommandLine);
    }

    // Cleanup the source dir of the temp files generated.
    {
        Msg("MapBuilder -> Deleting Temp files.\n");
        const char* rgpExt[] = { ".lin", ".ptr" };
        for (const auto* pExt : rgpExt)
        {
            char szWildcardDir[MAX_PATH];
            V_sprintf_safe(szWildcardDir, "%s\\%s\\*%s", gamedir, DIR_MAPS, pExt);
            g_pResourceCopy->DeleteDirRecursive(szWildcardDir, false, false);
        }
    }

    Msg("\n-------------------------------------------------------------------------------------------\n" );
    Msg("| MapBuilder -> Done in %.2fs | "                                      , Plat_FloatTime() - start);
    ColorSpewMessage(SPEW_MESSAGE, &ColorSucesfull, "All operation completed!"                            );
    Msg("\n-------------------------------------------------------------------------------------------\n" );
}


//-----------------------------------------------------------------------------
// Purpose:   Print mapbuilder usage
//-----------------------------------------------------------------------------
static void PrintUsage(int argc, char* argv[])
{
    Msg("\nUsage: mapbuilder.exe [options] <mapfile>\n\n" /*"-map <mapfile>\n\n"*/);
    ColorSpewMessage(SPEW_MESSAGE, &ColorHeader, " General Options:\n");
    Msg(//"   -map <path>:           Specify the path of the .vmf or .vmn file to compile.\n"
        "   -preset <preset_name>: Specify the preset name inside %s to be run. If specified not the default one will run.\n"
        "   -game <path>:          Specify the folder of the gameinfo.txt file.\n"
        "   -vproject <path>:      Same as \'-game\'.\n"
        "                          (e.g: \"C:\\Program Files (x86)\\Steam\\steamapps\\common\\Half-life 2\")\n"
        "  -insert_search_path <path>: Insert a search path.\n"
        "\n", FILE_MAPBUILDER_SETTINGS);
    ColorSpewMessage(SPEW_MESSAGE, &ColorHeader, " Building Options:\n");
    Msg("   -ignoreerrors:          Even if the .exe fails continue with the building.\n"
#if RESOURCECOMPILER_SYSTEM
        "   -all:                   Run all builders sequentially.\n"
        "   -world:                 Run the world builder.\n"
        "   -vis:                   Run the visibility builder.\n"
        "   -light:                 Run the lighting builder.\n"
        "   -cubemap:               Run the cubemap builder.\n"
        "   -nav:                   Runn the navigation builder.\n"    
        "   -ain:                   Runn the nodegraph builder.\n"  
#endif // RESOURCECOMPILER_SYSTEM
        "\n");
    ColorSpewMessage(SPEW_MESSAGE, &ColorHeader, " Spew Options:\n");
    Msg("   -v or -verbose:        Enables verbose.\n"
        "   -quiet:                Prints minimal text. (Note: Disables \'-verbose\' and \'-spewallcommands\')\n"  
        "\n");
    ColorSpewMessage(SPEW_MESSAGE, &ColorHeader, " Other Options:\n");
    Msg("   -help or -?:           Print usage."        
        "   -FullMinidumps:        Write large minidumps on crash.\n"
        "\n");

    DeleteCmdLine(argc, argv);
    CmdLib_Cleanup();
    CmdLib_Exit(-1);
}


//-----------------------------------------------------------------------------
// Purpose:   Parse command line
//-----------------------------------------------------------------------------
static void ParseCommandline(int argc, char* argv[])
{
    ColorSpewMessage(SPEW_MESSAGE, &ColorHeader, "Command Line:\n\t");
    for (std::size_t i = 1; i < argc; ++i)
    {
        Msg("%s ", argv[i]);
    }
    Msg("\n");

    if(argc == 1 || argc == 2)
        PrintUsage(argc, argv);

    for (int i = 1; i < argc; ++i)
    {
        V_FixSlashes(argv[i]);
        if (!V_stricmp(argv[i], "-?") || !V_stricmp(argv[i], "-help") || argc == 1)
        {
            PrintUsage(argc, argv);
        }
        else if (!V_stricmp(argv[i], "-v") || !V_stricmp(argv[i], "-verbose"))
        {
            verbose = true;
            g_eSpewMode = SpewMode::k_Verbose;
        } 
        else if (!V_stricmp(argv[i], "-quiet"))
        {
            g_eSpewMode = SpewMode::k_Quiet;
        } 
        else if (!V_stricmp(argv[i], "-FullMinidumps"))
        {
            EnableFullMinidumps(true);
        } 
        else if (!V_stricmp(argv[i], "-ignoreerrors"))
        {
            g_bIgnoreErrors = true;
        }
#if RESOURCECOMPILER_SYSTEM
        else if (!V_stricmp(argv[i], "-all"))
        {
            g_bRunWorld = true;
            g_bRunVis = true;
            g_bRunVrad = true;
            g_bRunMapInfo = true;
            g_bRunCubemap = true;
            g_bRunNav = true;
            g_bRunAin = true;
        } 
        else if (!V_stricmp(argv[i], "-world"))
        {
            g_bRunWorld = true;
        } 
        else if (!V_stricmp(argv[i], "-vis"))
        {
            g_bRunVis = true;
        } 
        else if (!V_stricmp(argv[i], "-light"))
        {
            g_bRunVrad = true;
        } 
        else if (!V_stricmp(argv[i], "-info"))
        {
            g_bRunMapInfo = true;
        } 
        else if (!V_stricmp(argv[i], "-cubemap"))
        {
            g_bRunCubemap = true;
        } 
        else if (!V_stricmp(argv[i], "-nav"))
        {
            g_bRunNav = true;
        } 
        else if (!V_stricmp(argv[i], "-ain"))
        {
            g_bRunAin = true;
        } 
#endif // RESOURCECOMPILER_SYSTEM
        /*
        else if (!V_stricmp(argv[i], "-map"))
        {
            if (++i < argc && argv[i][0] != '-')
            {
                const char* pMapPath = argv[i];
                if (!pMapPath)
                {
                    Error("\nError: \'-map\' requires a valid path argument. NULL path\n");
                }
                V_strcpy_safe(g_szSourceFile, pMapPath);
            }
            else
            {
                Error("\nError: \'-map\' requires a valid path argument.\n");
            }
        }
        */
        else if (!V_stricmp(argv[i], "-insert_search_path"))
        {
            // Pass to the commandline system
            i += 2;
        }
        else if (!V_stricmp(argv[i], "-preset"))
        {
            if (++i < argc && argv[i][0] != '-')
            {
                const char* pPresetName = argv[i];
                if (!pPresetName)
                    Error("\nError: \'-preset\' requires a valid string argument. NULL path\n");
                
                V_strcpy_safe(g_szPresetName, pPresetName);
                g_bDefaultPresetUser = true;
            }
            else
            {
                Error("\nError: \'-preset\' requires a valid path argument.\n");
            }
        }
        else if (!V_stricmp(argv[i], "-game") || !V_stricmp(argv[i], "-vproject"))
        {
            if (++i < argc && argv[i][0] != '-')
            {
                const char* gamePath = argv[i];
                if (!gamePath)
                {
                    Error("\nError: \'-game\' requires a valid path argument. NULL path\n");
                }
                V_strcpy_safe(gamedir, gamePath);
            }
            else
            {
                Error("\nError: \'-game\' requires a valid path argument.\n");
            }
        }
        else
        {
            if (i != argc - 1) 
            {
                Warning("\nWarning Unknown option \'%s\'\n", argv[i]);
                PrintUsage(argc, argv);
            }
        }
    }

    // Sanity check!
    if (gamedir[0] == '\0')
        Error("\nError: \'-game\' requires a valid path argument. NULL path\n");

    V_strcpy_safe(g_szSourceFile, argv[argc - 1]);

    Msg("\n");
}


//-----------------------------------------------------------------------------
// Purpose:     
//-----------------------------------------------------------------------------
static void Init(int argc, char* argv[])
{
    SetupDefaultToolsMinidumpHandler();
    CommandLine()->CreateCmdLine(argc, argv);
    InstallSpewFunction();
    ColorSpewMessage(SPEW_MESSAGE, &ColorHeader, "Unusuario2  - MapBuilder (Build: %s %s)\n", __DATE__, __TIME__);
    
    g_pResourceCopy = new CResourceCopy();
    
    ParseCommandline(argc, argv);
    CmdLib_InitFileSystem(gamedir);
    GetModuleFileNameA(NULL, g_szBaseToolDir, sizeof(g_szBaseToolDir));
    V_StripLastDir(g_szBaseToolDir, sizeof(g_szBaseToolDir));
    V_StripTrailingSlash(g_szBaseToolDir);
    V_StripTrailingSlash(gamedir);

    g_pFullFileSystem->GetSearchPath("BASE_PATH", false, g_szBaseGlobalDir, sizeof(g_szBaseGlobalDir));
    V_StripTrailingSlash(g_szBaseGlobalDir);

    // Basic info
    if (g_eSpewMode != SpewMode::k_Quiet) 
    {
        Msg("Working Directory:   ");   ColorSpewMessage(SPEW_MESSAGE, &ColorPath, "\"%s\"\n", gamedir);
        Msg("Tool Directory:      ");   ColorSpewMessage(SPEW_MESSAGE, &ColorPath, "\"%s\"\n", g_szBaseToolDir);
        Msg("Verbosity:           %s\n",
            []()->const char*
            {
                if (g_eSpewMode == SpewMode::k_VeryHighVerbose)
                    return "Very High";
                else if (g_eSpewMode == SpewMode::k_Verbose)
                    return "High";
                else if (g_eSpewMode == SpewMode::k_Normal)
                    return "Standard";
                else
                    return "Quiet";
            }()
                );
        Msg("\n");
    }
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
static void Destroy(int argc, char* argv[])
{
    delete g_pResourceCopy;
    
    DeleteCmdLine(argc, argv);
    CmdLib_Cleanup();
    CmdLib_Exit(0);
}


//-----------------------------------------------------------------------------
// Purpose:   Main funtion
//-----------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    Init(argc, argv);

    RunMapBuilder();

    Destroy(argc, argv);
	
    return 0;
}

