project(LightSpotVisualizer CXX)

################################################################################
# Source groups
################################################################################
set(SRC
    "LightSpotVisualizer.cpp"
)
source_group("Исходные файлы" FILES ${SRC})

set(ALL_FILES
    ${SRC}
)

################################################################################
# Target
################################################################################
add_executable(${PROJECT_NAME} ${ALL_FILES})

set(ROOT_NAMESPACE LightSpotVisualizer)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_GLOBAL_KEYWORD "Win32Proj"
)
################################################################################
# Output directory
################################################################################
set_target_properties(${PROJECT_NAME} PROPERTIES
RUNTIME_OUTPUT_DIRECTORY    "${CMAKE_CURRENT_SOURCE_DIR}/../../Workplace"
)

################################################################################
# Compile definitions
################################################################################
target_compile_definitions(${PROJECT_NAME} PRIVATE
    "FOR_LINUX;"
    "_CRT_SECURE_NO_WARNINGS;"
    "NDEBUG;"
    "_CONSOLE;"
    "UNICODE;"
    "_UNICODE"
)

################################################################################
# Compile and link options
################################################################################
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        /permissive-;
        /O2;
        /Oi;
        /sdl;
        ${DEFAULT_CXX_RUNTIME_LIBRARY};
        /Gy;
        /W3;
        ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
        ${DEFAULT_CXX_EXCEPTION_HANDLING};
        /Y-
    )
    target_link_options(${PROJECT_NAME} PRIVATE
        /NODEFAULTLIB:libboost_filesystem-vc140-mt-1_64.lib;
        /NODEFAULTLIB:libboost_system-vc140-mt-1_64.lib;
        /NODEFAULTLIB:libboost_chrono-vc140-mt-1_64.lib;
        /NODEFAULTLIB:libboost_date_time-vc140-mt-1_64.lib;
        /NODEFAULTLIB:libboost_thread-vc140-mt-1_64.lib;
        /DEBUG;
        /SUBSYSTEM:CONSOLE;
        /OPT:REF;
        /OPT:ICF;
        /INCREMENTAL:NO
    )
endif()
# Link with other targets.
target_link_libraries(${PROJECT_NAME} PUBLIC
        boost_system
        boost_filesystem
        boost_thread
        pthread
        sfml-window
        sfml-system
        sfml-graphics
        rt
        )

