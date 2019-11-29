#ifndef _MASON_FILE_IO_H_
#define _MASON_FILE_IO_H_

#include <stdio.h>

typedef struct File
{
	char* buffer;
	size_t size;
	size_t length;
} File;

extern long file_size(FILE* stream);

// Do not forget to free the file buffer after you're done
extern File file_read(const char* filename);

typedef struct LinearFileScanner
{
	File* file;
	size_t index;
} LinearFileScanner;

// Return the next character or '\0' if EOF was reached
extern char file_lfs_getchar(LinearFileScanner* lfs);

#endif