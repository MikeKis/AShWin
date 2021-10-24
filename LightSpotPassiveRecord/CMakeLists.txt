cmake_minimum_required(VERSION 3.5)

project(LightSpotPassiveRecord LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(LightSpotPassiveRecord main.cpp)

target_compile_definitions(LightSpotPassiveRecord PRIVATE FOR_LINUX)

################################################################################
# Dependencies
################################################################################
add_dependencies(${PROJECT_NAME}
    DVSEmulator
    LightSpotPassive
)

# Link with other targets.
target_link_libraries(${PROJECT_NAME} PUBLIC
    DVSEmulator
    LightSpotPassive
    boost_system
    boost_filesystem
    boost_thread
    pthread
    dl
)
