cmake_minimum_required(VERSION 3.14)

project(LightSpotPassiveClustering CXX)

add_library(${PROJECT_NAME} SHARED
  "LightSpotPassiveClustering.cpp"
  "../../pugixml-1.10/src/pugixml.cpp"
)
 
################################################################################
# Compile definitions
################################################################################
target_compile_definitions(${PROJECT_NAME} PRIVATE
    "FOR_LINUX;"
    "NDEBUG;"
    "_USRDLL;"
)

################################################################################
# Output directory
################################################################################
set_target_properties(${PROJECT_NAME} PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY    "${CMAKE_CURRENT_SOURCE_DIR}/../../Experiments"
        PREFIX ""
)
