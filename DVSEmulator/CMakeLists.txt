cmake_minimum_required(VERSION 3.14)

project(DVSEmulator LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#find_package(QT NAMES Qt6 Qt5 COMPONENTS Core REQUIRED)
#find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core REQUIRED)

add_library(DVSEmulator STATIC
  dvsemulator.cpp
  dvsemulator.h
)

#target_link_libraries(DVSEmulator PRIVATE Qt${QT_VERSION_MAJOR}::Core)

################################################################################
# Compile definitions
################################################################################
target_compile_definitions(${PROJECT_NAME} PRIVATE
    "FOR_LINUX;"
    "NDEBUG;"
    "_USRDLL;"
    "DVSEMULATOR_EXPORTS"
)


################################################################################
# Compile and link options
################################################################################
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        /O2;
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
    string(CONCAT FILE_CL_OPTIONS
        "/O2"
    )
    source_file_compile_options(GrowingCompletePNG.cpp ${FILE_CL_OPTIONS})
    target_link_options(${PROJECT_NAME} PRIVATE
        /SUBSYSTEM:WINDOWS;
        /OPT:REF;
        /OPT:ICF
    )
else()
    target_compile_options(${PROJECT_NAME} PRIVATE
            -fPIC
            )
endif()

