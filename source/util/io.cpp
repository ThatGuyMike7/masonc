#include <io.hpp>

#include <log.hpp>

#include <system_error>
#include <cstdlib>
#include <cstdio>
#include <filesystem>

namespace masonc
{
    path_type get_path_type(std::string* path)
    {
        if (path->length() >= 2 &&
           (*path)[path->length() - 2] == '/' &&
           (*path)[path->length() - 1] == '*')
        {
            path->pop_back();
            return DIR_PATH_RECURSE;
        }
        else if (path->length() >= 1 &&
                (*path)[path->length() - 1] == '/')
        {
            return DIR_PATH;
        }
        else
        {
            return FILE_PATH;
        }
    }

    std::vector<std::string> directory_files(const char* directory_path)
    {
        std::error_code error;
        auto directory_iterator = std::filesystem::directory_iterator(directory_path, error);

        if (error) {
            std::cout << "Directory " << directory_path << " does not exist." << std::endl;
            return std::vector<std::string>{};
        }

        std::vector<std::string> files;
        for (const auto& entry : directory_iterator) {
            files.push_back(entry.path().generic_string());
        }

        return files;
    }

    std::vector<std::string> directory_files(const char* directory_path,
        const std::unordered_set<std::string>& extensions)
    {
        std::error_code error;
        auto directory_iterator = std::filesystem::directory_iterator(directory_path, error);

        if (error) {
            std::cout << "Directory " << directory_path << " does not exist." << std::endl;
            return std::vector<std::string>{};
        }

        std::vector<std::string> files;
        for (const auto& entry : directory_iterator) {
            if (extensions.find(entry.path().extension().generic_string()) != extensions.end())
                files.push_back(entry.path().generic_string());
        }

        return files;
    }

	std::vector<std::string> directory_files_recurse(const char* directory_path)
	{
        std::error_code error;
        auto recursive_directory_iterator = std::filesystem::recursive_directory_iterator(directory_path, error);

        if (error) {
            std::cout << "Directory " << directory_path << " does not exist." << std::endl;
            return std::vector<std::string>{};
        }

        std::vector<std::string> files;
        for (const auto& entry : recursive_directory_iterator) {
			files.push_back(entry.path().generic_string());
		}

		return files;
    }

    std::vector<std::string> directory_files_recurse(const char* directory_path,
        const std::unordered_set<std::string>& extensions)
    {
        std::error_code error;
        auto recursive_directory_iterator = std::filesystem::recursive_directory_iterator(directory_path, error);

        if (error) {
            std::cout << "Directory " << directory_path << " does not exist." << std::endl;
            return std::vector<std::string>{};
        }

        std::vector<std::string> files;
        for (const auto& entry : recursive_directory_iterator) {
            if (extensions.find(entry.path().extension().generic_string()) != extensions.end())
                files.push_back(entry.path().generic_string());
        }

        return files;
    }

    std::vector<std::string> files_from_path(const path& path)
    {
        switch (path.type) {
            default:
                log_error("Not implemented \"path_type\" case.");
                return std::vector<std::string>{};
            case path_type::DIR_PATH_RECURSE:
                return directory_files_recurse(path.path_string.c_str());
            case path_type::DIR_PATH:
                return directory_files(path.path_string.c_str());
            case path_type::FILE_PATH:
                if (std::filesystem::exists(path.path_string))
                    return std::vector<std::string>{ path.path_string };
                else
                    return std::vector<std::string>{};
        }
    }

    std::vector<std::string> files_from_path(const path& path,
        const std::unordered_set<std::string>& extensions)
    {
        switch (path.type) {
            default: {
                log_error("Not implemented \"path_type\" case.");
                return std::vector<std::string>{};
            }
            case path_type::DIR_PATH_RECURSE: {
                return directory_files_recurse(path.path_string.c_str(), extensions);
            }
            case path_type::DIR_PATH: {
                return directory_files(path.path_string.c_str(), extensions);
            }
            case path_type::FILE_PATH: {
                std::filesystem::path _path{ path.path_string };

                if (std::filesystem::exists(_path) &&
                    extensions.find(_path.extension().generic_string()) != extensions.end())
                {
                    return std::vector<std::string>{ path.path_string };
                }
                else
                {
                    return std::vector<std::string>{};
                }
            }
        }
    }

	std::optional<char*> file_read(const char* path, const u64 block_size,
		u64* terminator_index)
	{
		u64 buffer_size = block_size;
		void* buffer = std::malloc(buffer_size);
		if (buffer == nullptr)
		{
			log_error(std::string{ "Unable to allocate memory buffer for file '" + std::string(path) + "'" }.c_str());
			return std::optional<char*>{};
		}

		#pragma warning (disable: 4996)
		std::FILE* stream = std::fopen(path, "r");
		if (stream == nullptr)
		{
			log_error(std::string{ "Unable to open stream for reading file '" + std::string(path) + "'" }.c_str());
			std::free(buffer);
			return std::optional<char*>{};
		}

		u64 bytes_read = std::fread(buffer, 1, block_size, stream);
		u64 total_bytes_read = bytes_read;

		while (bytes_read == block_size)
		{
			// Grow the buffer
			buffer_size += block_size;
			buffer = std::realloc(buffer, buffer_size);
			if(buffer == nullptr)
			{
				log_error(std::string{ "Unable to allocate memory buffer for file '" + std::string(path) + "'" }.c_str());
				return std::optional<char*>{};
			}

			// Location of last block (end of old buffer)
			char* location = static_cast<char*>(buffer);
			location += buffer_size - block_size;

			// Write to location
			bytes_read = std::fread(location, 1, block_size, stream);
			total_bytes_read += bytes_read;
		}

		// EOF was reached
		if (std::feof(stream) != 0)
		{
			// Allocate a block of memory to contain the string with a null terminator
			char* final_buffer = static_cast<char*>(std::malloc(total_bytes_read + 1));
			std::memcpy(final_buffer, buffer, total_bytes_read);

			//char* final_buffer_bytes = static_cast<char*>(final_buffer);
			final_buffer[total_bytes_read] = '\0';

			// Out the null terminator's index
			if(terminator_index != nullptr)
			{
				*terminator_index = total_bytes_read;
			}

			std::fclose(stream);
			std::free(buffer);
			return std::optional<char*>{ final_buffer };
		}

		// An error occured while reading
		if (std::ferror(stream) != 0)
		{
			log_error(std::string{ "Something went wrong while reading file '" + std::string(path) + "'" }.c_str());
			std::fclose(stream);
			std::free(buffer);
			return std::optional<char*>{};
		}

		log_error(std::string{ "EOF was not reached while reading file '" + std::string(path) + "'" }.c_str());
		std::fclose(stream);
		std::free(buffer);
		return std::optional<char*>{};
	}
}