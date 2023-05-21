#include "SysInfo.hpp"

#include <SDL_platform.h>

#include <cstring>
#include <stdio.h>

#if defined(__WINDOWS__)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <sys/utsname.h>
#endif

std::string obtainVersion()
{
    char version[256] = {};
#if defined(__WINDOWS__)
    const char unknown[] = "Unknown windows";
    const WCHAR *module_path = L"kernel32.dll";
    DWORD handle;
    DWORD size = GetFileVersionInfoSizeW(module_path, &handle);
    if (!size)
    {
        return unknown;
    }
    void *data = malloc(size);
    if (!GetFileVersionInfoW(module_path, handle, size, data))
    {
        free(data);
        return unknown;
    }
    VS_FIXEDFILEINFO *fileinfo;
    UINT unused;
    if (!VerQueryValueW(data, L"\\", (void **) &fileinfo, &unused))
    {
        free(data);
        return unknown;
    }
    snprintf(version,
             sizeof(version),
             "Windows %hu.%hu.%hu.%hu",
             HIWORD(fileinfo->dwProductVersionMS),
             LOWORD(fileinfo->dwProductVersionMS),
             HIWORD(fileinfo->dwProductVersionLS),
             LOWORD(fileinfo->dwProductVersionLS));
    free(data);
#else
    // Unix
    const char unknown[] = "Unknown unix";
    struct utsname u;
    if (uname(&u) != 0)
    {
        return unknown;
    }
    char extra[128];
    extra[0] = 0;

    do
    {
        auto fileHandle = fopen("/etc/os-release", "r");
        if (!fileHandle)
        {
            break;
        }

        char buf[4096];
        size_t read = fread(buf, 1, sizeof(buf) - 1, fileHandle);
        buf[read] = '\0';

        const char NameKey[] = "PRETTY_NAME=";
        const char* foundKey = strstr(buf, NameKey);
        if (foundKey)
        {
            const char* foundValue = foundKey + sizeof(NameKey);
            const char* end = strstr(foundValue, "\n");
            size_t length = sizeof(extra) - 1;
            if (end)
            {
                size_t recordLength = end - foundValue - 1;
                if (recordLength < length)
                {
                    length = recordLength;
                }
            }
            strncpy(extra, foundValue, length);
            extra[length] = '\0';
        }
    } while (false);

    if (extra[0])
    {
        snprintf(version, sizeof(version), "%s %s (%s, %s); %s", u.sysname, u.release, u.machine, u.version, extra);
    }
    else
    {
        snprintf(version, sizeof(version), "%s %s (%s, %s)", u.sysname, u.release, u.machine, u.version);
    }
#endif
    return std::string(version);
}

const std::string& getOsVersion()
{
    static std::string version = obtainVersion();
    return version;
}
