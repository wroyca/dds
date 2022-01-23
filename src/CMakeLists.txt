file(GLOB_RECURSE dds RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp *.h *.ui)

add_executable(dds
    ${dds}
)

target_include_directories(dds
PRIVATE
    "C:/Program Files/Microsoft Visual Studio/2022/Community/DIA SDK/include"
)

target_link_directories(dds
PRIVATE
    "C:/Program Files/Microsoft Visual Studio/2022/Community/DIA SDK/lib"
)

target_link_libraries(dds
PRIVATE
    Qt6::Widgets
    diaguids
)

set_target_virtual_directories(dds)
