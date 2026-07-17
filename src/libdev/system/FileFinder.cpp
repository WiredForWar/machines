/*******************************************************************
 *
 * filefind.cpp
 *
 * (c) Charybdis Limited, 1995. All Rights Reserved
 *
 *******************************************************************/

#include "system/FileFinder.hpp"
#include "system/FileData.hpp"
#include "system/FileTime.hpp"
#include "system/FileDate.hpp"

#include "ctl/Vector.hpp"

#ifdef _MSC_VER
#include "windows.h"
#else
#include <dirent.h>
#endif

SysFileFinder::SysFileFinder(const SysPathName& directory, const SysPathName& fileSpecification)
    : currentDirectory_(directory)
    , fileSpecification_(fileSpecification)
    , includeInSearch_(SysFileFinder::FILES)
{

    TEST_INVARIANT;

    POST(includedInSearch() == FILES);
}

SysFileFinder::~SysFileFinder()

{
    TEST_INVARIANT;
}

void SysFileFinder::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
    INVARIANT((includedInSearch() == FILES) || (includedInSearch() == DIRS) || (includedInSearch() == FILES_DIRS));
}

void SysFileFinder::fileSpecification(const SysPathName& fileSpec)
{
    fileSpecification_ = fileSpec;
}

void SysFileFinder::directory(const SysPathName& dir)
{
    currentDirectory_ = dir;
}

void SysFileFinder::examineSubdirectories(bool examine)
{
    examineSubdirectories_ = examine;
}

void SysFileFinder::find()
{
    processFiles(currentDirectory_);
}

void SysFileFinder::includeInSearch(SysFileFinder::SearchFor include)
{
    TEST_INVARIANT;

    includeInSearch_ = include;

    TEST_INVARIANT;
}

SysFileFinder::SearchFor SysFileFinder::includedInSearch() const
{
    TEST_INVARIANT;

    return includeInSearch_;
}

int SysFileFinder::processFiles(const SysPathName& directoryName)
{
#if defined _MSC_VER
    WIN32_FIND_DATA data;
    int err, abort;
    char pathname[_MAX_PATH];

    abort = 0;

    //  Look for matching files/dirs in this directory

    _makepath(pathname, NULL, directoryName.pathname().c_str(), fileSpecification_.pathname().c_str(), NULL);

    HANDLE hErr = FindFirstFile(pathname, &data);

    err = (hErr == INVALID_HANDLE_VALUE);

    while (!err && !abort)
    {
        FILETIME convTime;
        SYSTEMTIME sysTime;
        FileTimeToLocalFileTime(&data.ftLastWriteTime, &convTime);
        FileTimeToSystemTime(&convTime, &sysTime);

        SysFileTime ftime(
            sysTime.wHour, //  hours
            sysTime.wMinute, //  minutes
            sysTime.wSecond); //  seconds

        SysFileDate fdate(
            sysTime.wYear, //  year
            sysTime.wMonth, //  month
            sysTime.wDay); //  day of month

        // todo : fsize may overflow. This needs some investigation.
        SysFileData::SysFileSize fsize = (data.nFileSizeHigh * MAXDWORD) + data.nFileSizeLow;

        SysFileData fd(directoryName, data.cFileName, fdate, ftime, fsize);

        DWORD fileAttributes = GetFileAttributes(fd.pathName().c_str());

        bool dir = fileAttributes & FILE_ATTRIBUTE_DIRECTORY;
        bool incdirs = dir && ((includeInSearch_ == DIRS) || (includeInSearch_ == FILES_DIRS));
        bool incfile = !dir && ((includeInSearch_ == FILES) || (includeInSearch_ == FILES_DIRS));

        if ((incdirs || incfile) && strcmp(data.cFileName, ".") && // not interested in '.' or '..'
            strcmp(data.cFileName, ".."))
        {
            processFile(fd);
        }

        err = !FindNextFile(hErr, &data);
    }

    if (hErr != INVALID_HANDLE_VALUE)
        FindClose(hErr);

    if (examineSubdirectories_)
    {
        //  Recursively look for files/dirs in subdirectories

        _makepath(pathname, NULL, directoryName.pathname().c_str(), "*.*", NULL);

        hErr = FindFirstFile(pathname, &data);
        err = (hErr == INVALID_HANDLE_VALUE);

        while (!err && !abort)
        {
            SysPathName::Components comps;
            comps.reserve(2);

            comps.push_back(directoryName.pathname().c_str());
            comps.push_back(data.cFileName);
            SysPathName directory;
            directory.createFromComponents(comps);

            DWORD fileAttributes = GetFileAttributes(directory.pathname().c_str());

            if ((fileAttributes & FILE_ATTRIBUTE_DIRECTORY) && strcmp(data.cFileName, ".")
                && strcmp(data.cFileName, ".."))
            {
                _makepath(pathname, NULL, directoryName.pathname().c_str(), data.cFileName, NULL);

                abort = processFiles(pathname);
            }

            err = !FindNextFile(hErr, &data);
        }
    }

    if (hErr != INVALID_HANDLE_VALUE)
        FindClose(hErr);

    return (abort);
#elif defined _SDLAPP
    std::string dirName(directoryName.pathname());
    if (dirName[dirName.length() - 1] != '/')
        dirName += "/";

    struct dirent* dp;
    DIR* dir = opendir(dirName.c_str());

    // Unable to open directory stream
    if (!dir)
        return 0;

    while ((dp = readdir(dir)) != nullptr)
    {
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
        {
            // Construct new path from our base path
            std::string nextDirectoryName(dirName);
            nextDirectoryName += dp->d_name;

            if ((examineSubdirectories_ && !processFiles(nextDirectoryName)) || !examineSubdirectories_)
            {
                std::string filename(dp->d_name);
                std::string last = filename.substr(filename.length() - 3);
                if (last == fileSpecification_.extension())
                {
                    SysFileData fd(SysPathName(dirName), SysPathName(dp->d_name), SysFileDate(), SysFileTime(), 2);
                    processFile(fd);
                }
            }
        }
    }

    closedir(dir);
    return 1;
#else
#error Compiling system lib: one of _DOSAPP, _WIN95APP, etc. must be defined.
#endif
}
