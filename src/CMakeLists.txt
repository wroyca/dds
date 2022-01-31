file(GLOB_RECURSE dds RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp *.h *.ui)

add_executable(dds
    ${dds}
)

target_include_directories(dds
PRIVATE
    "C:/Program Files/Microsoft Visual Studio/2022/Community/DIA SDK/include"
    ${LIEF_INCLUDE_DIRS}
)

target_link_directories(dds
PRIVATE
    "C:/Program Files/Microsoft Visual Studio/2022/Community/DIA SDK/lib"
)

target_link_libraries(dds
PRIVATE
    Qt6::Widgets
    Zydis
    diaguids
    ${LIEF_LIBRARIES}
)

set_property(TARGET dds PROPERTY LINK_FLAGS /NODEFAULTLIB:MSVCRT)
set_target_virtual_directories(dds)
