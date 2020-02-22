# cmake -G "Visual Studio 15" -T "llvm" -D CMAKE_GENERATOR_PLATFORM=x64 -D CMAKE_BUILD_TYPE=Release ..
# cmake -G "Visual Studio 15" -D CMAKE_GENERATOR_PLATFORM=x64 -D CMAKE_BUILD_TYPE=Release ..
# cmake --build . --config Release

message(STATUS "CMake version is ${CMAKE_VERSION}")
if(${CMAKE_VERSION} VERSION_LESS "3.9.4") 
    message(STATUS "Please consider to switch to at least CMake 3.9.4")
endif()

cmake_minimum_required (VERSION 3.9.4)
project (masonc C)


# Emit compile commands
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


# Add header files
include_directories(${CMAKE_SOURCE_DIR}/ext/include ${CMAKE_SOURCE_DIR}/include)

# Add all .cpp files to SOURCES
# GLOB_RECURSE for subfolders, otherwise GLOB
file(GLOB_RECURSE SOURCES
    ${CMAKE_SOURCE_DIR}/ext/src/*.c
    ${CMAKE_SOURCE_DIR}/src/*.c
)

message(STATUS "Sources are ${SOURCES}")


# Find out build type and architecture
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(BUILD_TYPE "Debug")
else()
    set(BUILD_TYPE "Release")
endif()

set(BUILD_ARCHITECTURE "x64")

if(${CMAKE_SIZEOF_VOID_P} STREQUAL "4")
    message(ERROR "x86 architecture is not supported")
endif()


# Adjust library and output directories depending on build type and architecture
set(LIB_DIR ${CMAKE_SOURCE_DIR}/ext/lib/${BUILD_ARCHITECTURE})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/${BUILD_ARCHITECTURE})


# Find libraries depending on target architecture and debug/release
#find_library(GLFW_LIBRARIES
#    NAMES glfw3
#    PATHS ${LIB_DIR}
#    PATH_SUFIXES lib64 lib so a
#)

#find_library(LIBNOISE_LIBRARIES
#    NAMES libnoise
#    PATHS ${LIB_DIR}
#    PATH_SUFIXES lib64 lib so a
#)

#message(STATUS "Libraries are ${GLFW_LIBRARIES} ${LIBNOISE_LIBRARIES}")


#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")

#if(MSVC)
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z")
#    message(STATUS "Microsoft Visual Studio is used!")
#endif(MSVC)

add_executable(${PROJECT_NAME} ${SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED YES
    C_EXTENSIONS NO
)

#target_link_libraries(${PROJECT_NAME} ${GLFW_LIBRARIES} ${LIBNOISE_LIBRARIES})
#target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
