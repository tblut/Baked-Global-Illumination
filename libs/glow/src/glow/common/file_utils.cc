#include "file_utils.hh"

#ifdef _MSC_VER

#include <Windows.h>

int64_t glow::util::fileModificationTime(std::string const& filename)
{
    auto hFile = CreateFile(filename.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    FILETIME ftCreate, ftAccess, ftWrite;
    int64_t t = -1;
    if (GetFileTime(hFile, &ftCreate, &ftAccess, &ftWrite))
    {
        ULARGE_INTEGER ui;
        ui.HighPart = ftWrite.dwHighDateTime;
        ui.LowPart = ftWrite.dwLowDateTime;
        t = (int64_t)ui.QuadPart;
    }
    CloseHandle(hFile);
    return t;
}

#else

#include <sys/stat.h>
#include <sys/types.h>

int64_t glow::util::fileModificationTime(std::string const& filename)
{
    struct stat attr;
    stat(filename.c_str(), &attr);
#ifdef __APPLE__
    return attr.st_ctimespec.tv_sec * (int64_t)1000000000 + attr.st_ctimespec.tv_nsec;
#else
    return attr.st_ctim.tv_sec * (int64_t)1000000000 + attr.st_ctim.tv_nsec;
#endif
}
#endif
