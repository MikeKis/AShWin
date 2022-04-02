cmake_minimum_required(VERSION 3.15)
project(LightSpotVisualizer CXX)

################################################################################
# Target
################################################################################
add_executable(LightSpotVisualizer LightSpotVisualizer.cpp ../environment/LightSpotEnvironment.h)

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
    "NDEBUG;"
    "_CONSOLE"
)

################################################################################
# Compile and link options
################################################################################
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        /Od;
        /Oi;
        ${DEFAULT_CXX_RUNTIME_LIBRARY};
        /Gy;
        /W3;
        ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
        /wd4311;
        /wd4302;
        /wd4312;
        ${DEFAULT_CXX_EXCEPTION_HANDLING};
        /Y-
    )
    target_link_options(${PROJECT_NAME} PRIVATE
        /NODEFAULTLIB:libboost_filesystem-vc141-mt-1_64.lib;
        /NODEFAULTLIB:libboost_system-vc141-mt-1_64.lib;
        /NODEFAULTLIB:libboost_chrono-vc141-mt-1_64.lib;
        /NODEFAULTLIB:libboost_date_time-vc141-mt-1_64.lib;
        /NODEFAULTLIB:libboost_thread-vc141-mt-1_64.lib;
        /DEBUG;
        /SUBSYSTEM:CONSOLE;
        /OPT:REF;
        /OPT:ICF;
        /INCREMENTAL:NO
    )
else()
    target_compile_options(${PROJECT_NAME} PRIVATE
            -fpermissive
            )
endif()

################################################################################
# Dependencies
################################################################################

# Link with other targets.
target_link_libraries(${PROJECT_NAME} PUBLIC
    boost_system
    boost_filesystem
    boost_thread
    pthread
    rt
)
