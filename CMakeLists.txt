message(STATUS "CMake version is ${CMAKE_VERSION}")
if(${CMAKE_VERSION} VERSION_LESS "3.18.1")
    message(STATUS "At least CMake 3.18.1 is required")
endif()

cmake_minimum_required (VERSION 3.18.1)
project (masonc CXX)

# Add source files.
# "GLOB_RECURSE" for subfolders, otherwise "GLOB".
file(GLOB SOURCES
    "${CMAKE_SOURCE_DIR}/source/*.cpp"
    "${CMAKE_SOURCE_DIR}/source/lexer/*.cpp"
    "${CMAKE_SOURCE_DIR}/source/parser/*.cpp"
    "${CMAKE_SOURCE_DIR}/source/util/*.cpp"
    "${CMAKE_SOURCE_DIR}/source/language/*.cpp"
    "${CMAKE_SOURCE_DIR}/source/test/*.cpp"
)

# Determine configuration.
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(MASONC_CONFIG "debug")
elseif(${CMAKE_BUILD_TYPE} STREQUAL "Release")
    set(MASONC_CONFIG "release")
elseif(${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo")
    set(MASONC_CONFIG "release-with-debug-info")
elseif(${CMAKE_BUILD_TYPE} STREQUAL "MinSizeRel")
    set(MASONC_CONFIG "min-size-release")
else()
    set(MASONC_CONFIG "unsupported")
    message(FATAL_ERROR "Build configuration is unsupported")
endif()

# Determine architecture.
if(${CMAKE_SIZEOF_VOID_P} STREQUAL "8")
    set(MASONC_ARCH "64")
elseif(${CMAKE_SIZEOF_VOID_P} STREQUAL "4")
    set(MASONC_ARCH "32")
else()
    set(MASONC_ARCH "unsupported")
    message(FATAL_ERROR "Architecture is unsupported")
endif()

# Determine operating system.
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    set(MASONC_OS "win")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(MASONC_OS "linux")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    set(MASONC_OS "mac-os")
else()
    set(MASONC_OS ${CMAKE_SYSTEM_NAME})
    message(WARNING "Unexpected operating system name")
endif()

add_executable(${PROJECT_NAME} ${SOURCES})

# Add header files.
target_include_directories(${PROJECT_NAME}
    PRIVATE ${CMAKE_SOURCE_DIR}/source
    PRIVATE ${CMAKE_SOURCE_DIR}/source/lexer
    PRIVATE ${CMAKE_SOURCE_DIR}/source/parser
    PRIVATE ${CMAKE_SOURCE_DIR}/source/util
    PRIVATE ${CMAKE_SOURCE_DIR}/source/language
    PRIVATE ${CMAKE_SOURCE_DIR}/source/test
    PRIVATE ${CMAKE_SOURCE_DIR}/third-party/include
)

set(MASONC_OUTPUT_DIR ${CMAKE_SOURCE_DIR}/build/${MASONC_OS}-${MASONC_ARCH}/${MASONC_CONFIG})

# Specify output directory, executable name and C++ standard.
set_target_properties(${PROJECT_NAME} PROPERTIES
    # "$<0:>" is a generator expression, it prevents multi-configuration generators
    # from appending a per-configuration sub-directory to the specified path.
    RUNTIME_OUTPUT_DIRECTORY ${MASONC_OUTPUT_DIR}/$<0:>
    OUTPUT_NAME "masonc"
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

# Set warning level.
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        /W4     # Warning level.
        /wd4458 # Suppress warning "declaration of 'x' hides class member".
        #/WX    # Treat warnings as errors.
    )
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic -Werror)
endif()

# Use multiple processes to build faster with Visual Studio.
if(CMAKE_GENERATOR MATCHES "Visual Studio")
    target_compile_options(${PROJECT_NAME} PRIVATE /MP)
endif()

# Define "MASONC_DEBUG" only in Debug mode.
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    target_compile_definitions(${PROJECT_NAME} PRIVATE MASONC_DEBUG)
endif()

set(MASONC_LIB_DIR ${CMAKE_SOURCE_DIR}/third-party/lib)

find_library(MASONC_LLVM_LIB
    NAMES LLVM-C
    PATHS ${MASONC_LIB_DIR}
    PATH_SUFFIXES lib64 lib so a
)

#message(STATUS "Sources are ${SOURCES}")
#message(STATUS "Libraries are ${MASONC_LLVM_LIB}")

target_link_libraries(${PROJECT_NAME} ${MASONC_LLVM_LIB})

#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")

#if(MSVC)
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z")
#    message(STATUS "Microsoft Visual Studio is used")
#endif(MSVC)

#target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
