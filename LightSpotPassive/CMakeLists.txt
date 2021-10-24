cmake_minimum_required(VERSION 3.15)
project(LightSpotPassive CXX)

################################################################################
# Source groups
################################################################################
set(Source_Files
    "LightSpotPassive.cpp"
)
source_group("Source Files" FILES ${Source_Files})

set(ALL_FILES
    ${Source_Files}
)

################################################################################
# Target
################################################################################
add_library(${PROJECT_NAME} SHARED ${ALL_FILES})

set(ROOT_NAMESPACE LightSpotPassive)

################################################################################
# Output directory
################################################################################
set_target_properties(${PROJECT_NAME} PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY    "${CMAKE_CURRENT_SOURCE_DIR}/../Workplace"
        PREFIX ""
)

################################################################################
# Compile definitions
################################################################################
target_compile_definitions(${PROJECT_NAME} PRIVATE
    "FOR_LINUX;"
    "NDEBUG;"
    "_USRDLL;"
    "LIGHTSPOTPASSIVE_EXPORTS"
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
        /SUBSYSTEM:WINDOWS;
        /OPT:REF;
        /OPT:ICF;
        /INCREMENTAL:NO
    )
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC
        boost_system
        boost_filesystem
        boost_thread
        )

