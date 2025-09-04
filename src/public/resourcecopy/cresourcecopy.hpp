//=== CResourceCopy -> Written by Unusuario2, https://github.com/Unusuario2  ====//
//
// Purpose:
//
// $NoKeywords: $
//===============================================================================//
#ifndef CRESOURCECOPY_H
#define CRESOURCECOPY_H

#ifdef _WIN32
#pragma once
#endif // _WIN32

#include <array>
#include <vector>
#include <mutex>
#include <pipeline_shareddefs.h>


//-----------------------------------------------------------------------------
// Purpose: Generic data containers
//-----------------------------------------------------------------------------
using ContainerString           = std::array<char, MAX_PATH>;
using ContainerList             = std::vector<ContainerString>;
using ContainerStringExtended   = std::array<char, MAX_CMD_BUFFER_SIZE>;
using ContainerListExtended     = std::vector<ContainerStringExtended>;


//-----------------------------------------------------------------------------
// Purpose: Specific File containers
//-----------------------------------------------------------------------------
using FileString                = ContainerString;
using FileStringExtended        = ContainerStringExtended;
using FileList                  = ContainerList;
using FileListExtended          = ContainerListExtended;


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
class CResourceCopy
{
private:
    float       m_flTime;
    int         m_iThreads;
    int         m_iCompletedOperations;
    int         m_iSkippedOperations;
    int         m_iFailedOperations;
    SpewMode    m_eSpewMode;
    std::mutex  m_MsgMutex;

private:
    void SetLogicalProcessorCount();
    FileList ScanDirectoryWorker(const char* baseDir, bool fullScan, const char* pExtFilter);

public:
    CResourceCopy::CResourceCopy();

    // Single File Operations
    bool CopyFileTo(const char* pSrcPath, const char* pDstPath, const bool bOverwrite);
    bool TransferFileTo(const char* pSrcPath, const char* pDstPath, const bool bOverwrite);
    bool DeleteFileIn(const char* pDir);
    bool DirExist(const char* pszDir);
    bool FileExist(const char* pszFile);
    bool DeleteEmptyFolder(const char* pszDir);
    bool CreateDir(const char* pszDir);
    bool IsWritable(const char* pSrcDir, const char* pDstDir, const bool bIsPath);

    // Batch File Operations (Note: They support multithreading and wildcards!)
    void CopyDirTo(const char* pSrcDir, const char* pDstDir, const bool bMultiThread = true, const bool bOverwrite = true, const FileList* pCopyList = nullptr);
    void TransferDirTo(const char* pSrcDir, const char* pDstDir, const bool bMultiThread, const bool bDeleteMainFolder = false, const bool bOverwrite = true, const FileList* pDeleteList = nullptr);
    void DeleteDirRecursive(const char* pDir, const bool bMultiThread = true, const bool bDeleteMainFolder = false, const FileList* pDeleteList = nullptr);
    FileList ScanDirectoryRecursive(const char* pszDir);

    // Sanity checks
    std::size_t GetFileSizeFast(const char* pszFile);
    std::size_t GetDriveFreeSpace(const char* folderPath);
    std::size_t GetFolderSize(const char* inputPath);

    // Misc
    void PrintDirContents(const char* pDir);
    void GenerateGlobalOperationReport();
    void GenerateHardwareReport(const char* pSrc, const char* pDst, const bool bIsPath);
    void GenerateErrorReport();
    void SetThreads(const int iThreads);
    void SetVerboseSpewMode();
    void SetNormalSpewMode();
    void SetQuietSpewMode();
};


#endif // CRESOURCECOPY_H

