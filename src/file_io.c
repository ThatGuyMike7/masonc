#include <file_io.h>

#include <common.h>

#include <stdlib.h>
#include <string.h>
#include <errno.h>

long file_size(FILE* stream)
{
	fseek(stream, 0, SEEK_END);
	long size = ftell(stream);
	fseek(stream, 0, SEEK_SET);
	return size;
}

File file_read(const char* filename)
{
	FILE* stream = fopen(filename, "r");
	if(stream == NULL)
		quit(strerror(errno));
	
	File file;
	
	file.size = file_size(stream);
	file.buffer = malloc(file.size);
	size_t result = fread(file.buffer, 1, file.size, stream);
	
	// Either error occured or EOF was reached while reading
	if(result != file.size)
	{
		if(feof(stream) != 0)
		{
			// EOF was reached -> Success
			file.length = result;
			fclose(stream);
			return file;
		}
		if(ferror(stream) != 0)
			quit("Error while reading file stream");
	}
	
	quit("Error: EOF was not reached while reading file stream");
	return file;
}

char file_lfs_getchar(LinearFileScanner* lfs)
{
	if(lfs->index == lfs->file->length)
		return EOF;
		
	char c = lfs->file->buffer[lfs->index];
	lfs->index += 1;
	return c;
}