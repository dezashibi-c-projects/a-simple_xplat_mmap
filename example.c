// ***************************************************************************************
//    Project: Easy Cross-Platform File Mapping with a Single-Header C Library
//    File: example.c
//    Date: 2024-08-09
//    Author : Navid Dezashibi
//    Contact: navid@dezashibi.com
//    Website: https://www.dezashibi.com | https://github.com/dezashibi
//    License:
//     Please refer to the LICENSE file, repository or website for more information about
//     the licensing of this work. If you have any questions or concerns,
//     please feel free to contact me at the email address provided above.
// ***************************************************************************************
// *  Description: Refer to readme file
// ***************************************************************************************

#define DMMAP_IMPL

#include "dmmap.h"
#include <stdio.h>

int main()
{
    // Open and map the file into memory in read-only mode
    DmmapFile file = dmmap_file_open("bench_text.txt", 1);

    if (file.data)
    {
        // Successfully mapped the file
        printf("File size: %zu bytes\n", file.size); // Print the size of the file

        // Access and print the contents of the file

        /* Uncomment the section below if you need to, commented out due to large content of the file */

        // char* content = (char*)file.data; // Treat the file data as a string
        // for (size_t i = 0; i < file.size; ++i)
        // {
        //     putchar(content[i]); // Print each character
        // }

        // Close and unmap the file, releasing resources
        dmmap_file_close(&file);
    }
    else
    {
        // Failed to map the file, print an error message
        printf("Failed to map file\n");
    }

    return 0;
}
