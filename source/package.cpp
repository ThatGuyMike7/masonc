#include "package.hpp"

namespace masonc
{
    std::string module_name_from_package_name(const std::string& package_name)
    {
        size_t dot_index = package_name.find('.');
        
        if(dot_index == std::string::npos)
            return package_name;
        else
            return package_name.substr(0, dot_index);
    }
}