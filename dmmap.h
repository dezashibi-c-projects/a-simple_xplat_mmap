// ***************************************************************************************
//    Project: Easy Cross-Platform File Mapping with a Single-Header C Library
//    Repository: https://github.com/dezashibi-c/dmmap
//    File: dmmap.h
//    Date: 2024-08-09
//    Author: Navid Dezashibi
//    Contact: navid@dezashibi.com
//    Website: https://www.dezashibi.com | https://github.com/dezashibi
//    License:
//        BSD 3-Clause License
//
//        Copyright (c) 2024, Navid Dezashibi <navid@dezashibi.com> All rights reserved.
//
//        Redistribution and use in source and binary forms, with or without
//        modification, are permitted provided that the following conditions are met:
//
//        1. Redistributions of source code must retain the above copyright notice,
//           this list of conditions and the following disclaimer.
//
//        2. Redistributions in binary form must reproduce the above copyright notice,
//           this list of conditions and the following disclaimer in the documentation
//           and/or other materials provided with the distribution.
//
//        3. Neither the name of the copyright holder nor the names of its contributors
//           may be used to endorse or promote products derived from this software
//           without specific prior written permission.
//
//        THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
//        ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//        WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
//        IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
//        INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
//        BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//        DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//        LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
//        OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
//        OF THE POSSIBILITY OF SUCH DAMAGE.
// ***************************************************************************************
// *  Description:
// *      This header file provides a cross-platform implementation of memory-mapped
// *      file access, supporting Windows, Linux, and macOS. It abstracts platform-
// *      specific APIs into a unified interface, allowing for efficient file manipulation
// *      directly in memory. The following steps outline how to use this library in your
// *      project:
// *
// *      1. Include the "dmmap.h" header file in your project.
// *
// *      2. Define the macro "DMMAP_IMPL" in one of your source files to enable the
// *         implementation of the library, like so:
// *
// *         #define DMMAP_IMPL
// *         #include "dmmap.h"
// *
// *      3. Use the `dmmap_file_open` function to map a file into memory. This function
// *         returns a `DmmapFile` structure containing a pointer to the file’s contents
// *         in memory and its size. Example:
// *
// *         DmmapFile mapped_file = dmmap_file_open("example.txt", 1);
// *         if (mapped_file.data == NULL) {
// *             // Handle error: The file could not be opened or mapped.
// *         }
// *
// *      4. Access the file’s contents directly via the `data` pointer in the `DmmapFile`
// *         structure. You can manipulate the file contents as needed. Example:
// *
// *         char* content = (char*)mapped_file.data;
// *         for (size_t i = 0; i < mapped_file.size; ++i) {
// *             putchar(content[i]); // Print each character in the file
// *         }
// *
// *      5. Once done, call `dmmap_file_close` to unmap the file and release associated
// *         resources, like so:
// *
// *         dmmap_file_close(&mapped_file);
// *
// *      This library simplifies cross-platform development by providing a consistent
// *      interface for memory-mapped file operations, making your file I/O tasks more
// *      efficient and easier to manage.
// ***************************************************************************************

#ifndef DMMAP__H__
#define DMMAP__H__

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @struct DmmapFile
     * @brief A structure representing a memory-mapped file.
     *
     * The `DmmapFile` structure is used to hold the essential information
     * about a file that has been mapped into the memory address space of
     * the process. This structure includes:
     *
     * - `data`: A pointer to the start of the memory-mapped file contents.
     * - `size`: The size of the memory-mapped file in bytes.
     * - `fd`: A file descriptor (on POSIX systems) or a file mapping handle
     *         (on Windows). The `uintptr_t` type ensures portability across platforms.
     *
     * This structure is the main interface for interacting with the memory-mapped
     * file in the library, allowing direct access to the file contents as if they
     * were part of the process's memory.
     */
    typedef struct DmmapFile
    {
        void* data;   /**< Pointer to the memory-mapped file contents */
        size_t size;  /**< Size of the memory-mapped file in bytes */
        uintptr_t fd; /**< File descriptor or file mapping handle */
    } DmmapFile;

    /**
     * @brief Maps a file into memory, providing direct access to its contents.
     *
     * The `dmmap_file_open` function opens a file and maps it into the memory
     * address space of the process. The function returns a `DmmapFile` structure
     * containing a pointer to the file's contents, the size of the file, and a
     * file descriptor or mapping handle. The function works across different
     * platforms by abstracting platform-specific details.
     *
     * @param filename The path to the file to be mapped.
     * @param read_only A flag indicating whether the file should be mapped as
     *                  read-only (1) or read-write (0).
     * @return A `DmmapFile` structure with the mapped file information. If the
     *         mapping fails, the `data` field in the returned structure will be `NULL`.
     *
     * @note If the file cannot be opened or mapped, the `data` field in the returned
     *       `DmmapFile` structure will be `NULL`, and appropriate error handling should
     *       be performed.
     */
    DmmapFile dmmap_file_open(const char* filename, int read_only);

    /**
     * @brief Unmaps a file from memory and releases associated resources.
     *
     * The `dmmap_file_close` function unmaps a memory-mapped file from the process's
     * memory address space and releases any resources associated with it, such as
     * file descriptors or mapping handles. This function should be called once you
     * are done working with the mapped file to prevent memory leaks or file locking issues.
     *
     * @param file A pointer to the `DmmapFile` structure representing the memory-mapped file.
     *             After this function is called, the `data` field in the structure will be set to `NULL`,
     *             and the other fields will be reset.
     */
    void dmmap_file_close(DmmapFile* file);

#ifdef __cplusplus
}
#endif

#ifdef DMMAP_IMPL

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

#endif // DMMAP__H__
