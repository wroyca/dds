cmake_minimum_required(VERSION 3.22)
project(dds LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Turn on automatic invocation of the MOC, UIC & RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Find the QtWidgets library
find_package(Qt6 REQUIRED COMPONENTS Widgets)

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
