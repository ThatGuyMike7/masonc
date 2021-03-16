#ifndef MASONC_IO_HPP
#define MASONC_IO_HPP

#include <common.hpp>

#include <vector>
#include <string>

namespace masonc
{
    enum path_type : u8
    {
        FILE_PATH,
        DIR_PATH,
        DIR_PATH_RECURSE
    };

    // Returns "DIR_PATH_RECURSE" if "path" ends with "/*" and
    // pops the last character so it becomes a valid directory path.
    //
    // Returns "DIR_PATH" if "path" ends with "/".
    // Otherwise returns "FILE_PATH".
    path_type get_path_type(std::string* path);

    struct path
    {
        std::string path_string;
        path_type type;

        path(const std::string& path_string) {
            this->path_string = path_string;
            this->type = get_path_type(&this->path_string);
        }

        //operator std::string() const { return path_string; }
        //operator const char*() const { return path_string.c_str(); }
    };

    // Receive a list of all files in a directory.
    std::vector<std::string> directory_files(const char* directory_path);
    std::vector<std::string> directory_files(const char* directory_path,
        const std::vector<std::string>& extensions);

    // Receive a list of all files in a directory and all its sub-directories.
    std::vector<std::string> directory_files_recurse(const char* directory_path);
    std::vector<std::string> directory_files_recurse(const char* directory_path,
        const std::vector<std::string>& extensions);

    // Receive a list of file paths from a path. All files are guaranteed to exist.
    //
    // If "path.type" is "DIR_PATH_RECURSE", all files in the directory and its sub-directories are returned.
    // If "path.type" is "DIR_PATH", all files in the directory are returned.
    // If "path.type" is "FILE_PATH", only the file path is returned if the file exists.
    // If it does not exist, the returned vector is empty.
    std::vector<std::string> files_from_path(const path& path);
    std::vector<std::string> files_from_path(const path& path,
        const std::vector<std::string>& extensions);

	// Read a file into a buffer. If 'terminator_index' is not nullptr,
    // it will be set to the index of the null terminator (last byte in array).
	// std::free() has to be called on the buffer once it is not used anymore.
    // Returns "nullptr" if something went wrong.
	char* file_read(const char* path, const u64 block_size = 64000,
        u64* terminator_index = nullptr);
}

#endif