cmake_minimum_required(VERSION 3.22)
project(dds LANGUAGES CXX)

# Turn on Debug build by default
set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)

# Register C++20 hard requirements
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Turn on automatic invocation of the MOC, UIC & RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Register Qt6 dependency
find_package(Qt6 REQUIRED COMPONENTS Widgets)

# Turn off Zydis tools and examples
option(ZYDIS_BUILD_TOOLS "" OFF)
option(ZYDIS_BUILD_EXAMPLES "" OFF)

# Register Zydis dependency
add_subdirectory("externals/zydis")

# Set LIEF install directory
set(LIEF_DIR ${CMAKE_SOURCE_DIR}/externals/LIEF)
list(APPEND CMAKE_MODULE_PATH ${LIEF_DIR}/share/LIEF/cmake)
include(FindLIEF)

# Register LIEF dependency
find_package(LIEF REQUIRED COMPONENTS STATIC)

# Create virtual directories in Visual Studio
function(set_target_virtual_directories target_name)
    get_target_property(target_sources  "${target_name}" SOURCES)
    foreach(file_name IN LISTS target_sources)
        get_filename_component(dir_name "${file_name}" PATH)
        string(REPLACE "/" "\\" group_name "${dir_name}")
        source_group("${group_name}" FILES "${file_name}")
    endforeach()
endfunction()

# Enables relative paths for MSBuild projects
include_directories(.)

add_compile_options(
    /MP                     # Enables parallel (aka multiprocessor, aka multi-core) builds
    $<$<CONFIG:Debug>:/Zi>  # Produces symbolic debugging information
)

add_subdirectory(src)
