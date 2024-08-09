// ***************************************************************************************
//    Project: Easy Cross-Platform File Mapping with a Single-Header C Library
//    File: deza_mmap.h
//    Date: 2024-08-09
//    Author: Navid Dezashibi
//    Contact: navid@dezashibi.com
//    Website: https://www.dezashibi.com | https://github.com/dezashibi
//    License:
//     MIT License
//
//     Copyright (c) 2024 Navid Dezashibi
//
//     Permission is hereby granted, free of charge, to any person obtaining a copy of this
//     software and associated documentation files (the "Software"), to deal in the Software
//     without restriction, including without limitation the rights to use, copy, modify,
//     merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
//     permit persons to whom the Software is furnished to do so, subject to the following
//     conditions:
//
//     The above copyright notice and this permission notice shall be included in all copies
//     or substantial portions of the Software.
//
//     THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
//     INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
//     PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
//     HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
//     CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
//     OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
// ***************************************************************************************
// *  Description:
// *  This header file provides a cross-platform implementation of memory-mapped file
// *  access, supporting Windows, Linux, and macOS. It abstracts platform-specific APIs
// *  into a simple, unified interface for mapping files into memory, allowing for easy
// *  file manipulation in C projects.
// ***************************************************************************************

#ifndef DEZA_MMAP__H__
#define DEZA_MMAP__H__

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct DmmapFile
    {
        void* data;
        size_t size;
        uintptr_t fd;
    } DmmapFile;

    DmmapFile dmmap_file_open(const char* filename, int read_only);

    void dmmap_file_close(DmmapFile* file);

#ifdef __cplusplus
}
#endif

#ifdef DEZA_MMAP_IMPL

#ifdef _WIN32
#include <windows.h>

DmmapFile dmmap_file_open(const char* filename, int read_only)
{
    DmmapFile result = {0};
    DWORD access = read_only ? GENERIC_READ : GENERIC_READ | GENERIC_WRITE;
    DWORD protect = read_only ? PAGE_READONLY : PAGE_READWRITE;
    DWORD map_access = read_only ? FILE_MAP_READ : FILE_MAP_WRITE;

    HANDLE file = CreateFileA(filename, access, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE)
        return result;

    DWORD fileSize = GetFileSize(file, NULL);
    HANDLE map = CreateFileMapping(file, NULL, protect, 0, fileSize, NULL);
    if (!map)
    {
        CloseHandle(file);
        return result;
    }

    void* data = MapViewOfFile(map, map_access, 0, 0, fileSize);
    if (!data)
    {
        CloseHandle(map);
        CloseHandle(file);
        return result;
    }

    result.data = data;
    result.size = fileSize;
    result.fd = (uintptr_t)map; // Storing map handle as uintptr_t
    CloseHandle(file);
    return result;
}

void dmmap_file_close(DmmapFile* file)
{
    if (file->data)
    {
        UnmapViewOfFile(file->data);
        CloseHandle((HANDLE)file->fd);
        file->data = NULL;
        file->size = 0;
        file->fd = 0;
    }
}

#else // POSIX (Linux, macOS)
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

DmmapFile dmmap_file_open(const char* filename, int read_only)
{
    DmmapFile result = {0};
    int flags = read_only ? O_RDONLY : O_RDWR;
    int fd = open(filename, flags);
    if (fd == -1)
        return result;

    struct stat sb;
    if (fstat(fd, &sb) == -1)
    {
        close(fd);
        return result;
    }

    int prot = read_only ? PROT_READ : PROT_READ | PROT_WRITE;
    void* data = mmap(NULL, sb.st_size, prot, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED)
    {
        close(fd);
        return result;
    }

    result.data = data;
    result.size = sb.st_size;
    result.fd = fd;
    return result;
}

void dmmap_file_close(DmmapFile* file)
{
    if (file->data)
    {
        munmap(file->data, file->size);
        close(file->fd);
        file->data = NULL;
        file->size = 0;
        file->fd = 0;
    }
}

#endif

#endif

#endif // DEZA_MMAP__H__
