cmake_minimum_required(VERSION 3.15)
project(LightSpot CXX)

################################################################################
# Source groups
################################################################################
set(Source_Files
    "LightSpot.cpp"
    "../../../pugixml-1.10/src/pugixml.cpp"
)
source_group("Source Files" FILES ${Source_Files})

set(ALL_FILES
    ${Source_Files}
)

################################################################################
# Target
################################################################################
add_library(${PROJECT_NAME} SHARED ${ALL_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "networks")

set(ROOT_NAMESPACE LightSpot)

################################################################################
# Output directory
################################################################################
set_target_properties(${PROJECT_NAME} PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY    "${CMAKE_CURRENT_SOURCE_DIR}/../../../Experiments"
        PREFIX ""
)

################################################################################
# Compile definitions
################################################################################
target_compile_definitions(${PROJECT_NAME} PRIVATE
    "FOR_LINUX;"
    "NDEBUG;"
    "_USRDLL;"
    "LIGHTSPOT_EXPORTS"
)


################################################################################
# Dependencies
################################################################################
target_link_libraries(${PROJECT_NAME} PUBLIC
        boost_system
        boost_filesystem
        boost_thread
        )

