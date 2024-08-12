# Easy Cross-Platform File Mapping with a Single-Header C Library

Memory-mapped file access is a powerful technique that allows a file to be mapped into the memory address space of a process, enabling efficient file I/O operations. By treating a file as if it were a part of the process’s memory, you can manipulate file contents directly using pointers, avoiding the need for repetitive read/write system calls. However, memory-mapped file access differs across platforms like Windows, Linux, and macOS, requiring platform-specific APIs. To address this, I've implemented a cross-platform, single-header C library that abstracts these differences into a simple and unified interface.

## Defining the Interface

The first step in creating the library was to define a consistent interface that would work across all supported platforms. The main structure in this interface is `DmmapFile`, which holds the file data pointer, the size of the file, and a file descriptor or handle:

```c
typedef struct DmmapFile
{
    void* data;
    size_t size;
    uintptr_t fd;
} DmmapFile;
```

The `data` field points to the memory-mapped file contents, `size` stores the size of the mapped file, and `fd` holds the file descriptor (on POSIX systems) or the file mapping handle (on Windows). The `uintptr_t` type is used for `fd` to ensure it can hold any pointer or integer type, making it portable across platforms.

## Implementing the Functionality

The core functionality is provided by two functions: `dmmap_file_open` and `dmmap_file_close`.

**Windows Implementation:**

On Windows, memory-mapped file access involves several steps. First, the file is opened using `CreateFileA`, and then a file mapping object is created with `CreateFileMapping`. Finally, the file's contents are mapped into memory using `MapViewOfFile`. Here is the implementation:

```c
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
```

In the `dmmap_file_open` function, the file is opened and mapped into memory. The file mapping handle is stored in `fd`, and the file handle is closed since it's no longer needed after mapping. The `dmmap_file_close` function unmaps the memory and closes the file mapping handle.

**POSIX Implementation:**

For POSIX-compliant systems (Linux, macOS), the implementation is slightly different but follows a similar flow. The file is opened using `open`, and its size is determined with `fstat`. The file is then memory-mapped using `mmap`, and the resulting pointer and file descriptor are stored in the `DmmapFile` structure:

```c
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
```

The `dmmap_file_close` function unmaps the file and closes the file descriptor, cleaning up the resources.

## Cross-Platform Integration

By using conditional compilation with `#ifdef _WIN32`, the code automatically selects the appropriate implementation depending on the target platform. This allows the same interface to work seamlessly on Windows, Linux, and macOS.

```c
#ifdef _WIN32
    // Windows-specific implementation
#else
    // POSIX-specific implementation
#endif
```

This approach not only simplifies cross-platform development but also ensures that the code remains clean and maintainable.

## Using the Cross-Platform Memory-Mapped File Access Library

**👉 NOTE: You can get the the latest for the latest `rc` or `beta` and/or get the latest stable version from releases**

Using the memory-mapped file access library in your C projects is straightforward. The library is designed to be minimalistic and easy to integrate.

1. To begin using the library, you simply need to include the `dmmap.h` header file in your project and define the `DMMAP_IMPL` macro in one of your source files to enable the implementation.

```c
#define DMMAP_IMPL
#include "dmmap.h"
```

This step is essential as it compiles the implementation code into your project, linking it with the rest of your codebase.

2. Once you have set up the header and implementation, you can easily open a file and map it into memory using the `dmmap_file_open` function. This function takes the file path and a `read_only` flag, which indicates whether the file should be opened in read-only mode or read-write mode. Here’s an example:

```c
DmmapFile mapped_file = dmmap_file_open("example.txt", 1);
if (mapped_file.data == NULL) {
    // Handle error: The file could not be opened or mapped.
}
```

`"example.txt"` is opened in read-only mode. The `dmmap_file_open` function returns a `DmmapFile` structure that contains the pointer to the file’s contents in memory (`data`) and the size of the file (`size`). If the `data` field is `NULL`, it indicates that the file could not be opened or mapped, and appropriate error handling should be performed.

3. After successfully mapping a file, you can access its contents directly through the `data` pointer. This allows you to treat the file’s contents as if they were part of the process’s memory, enabling efficient manipulation:

```c
char* content = (char*)mapped_file.data;
for (size_t i = 0; i < mapped_file.size; ++i) {
    putchar(content[i]); // Print each character in the file
}
```

The file contents are treated as a string of characters, and each character is printed out using `putchar`.

4. When you're done working with the memory-mapped file, it's important to release the resources by calling `dmmap_file_close`. This function unmaps the file from memory and closes any associated file descriptors or handles:

```c
dmmap_file_close(&mapped_file);
```

This ensures that all resources are properly freed, preventing memory leaks or file locking issues.

**👉 Now** you can easily integrate the memory-mapped file access library into your C projects. The library simplifies cross-platform development by providing a consistent and efficient way to work with files directly in memory, regardless of the underlying operating system. With just a few lines of code, you can open, manipulate, and close memory-mapped files, making your file I/O operations more efficient and easier to manage.

## Conclusion

This cross-platform memory-mapped file access library abstracts away the differences between Windows and POSIX systems, providing a unified interface for memory-mapped file operations. By using this single-header library, developers can easily integrate memory-mapped file access into their C projects without worrying about platform-specific details. The library is efficient, easy to use, and fully portable, making it a valuable tool for any C programmer dealing with file I/O operations.

## License for `dmmap.h`

BSD 3-Clause License

Please refer to [LICENSE](/LICENSE) file.
