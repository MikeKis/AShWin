cmake_minimum_required(VERSION 3.14)
#set(CMAKE_CXX_COMPILER "/usr/bin/g++-6")
#set(CMAKE_CUDA_COMPILER "/usr/local/cuda/bin/nvcc")

project(AShWin)

set(CMAKE_CXX_STANDARD 14)
#set(CMAKE_CUDA_FLAGS "-ccbin g++-6")

include_directories(.)
include_directories(DVSEmulator)
link_directories(/usr/local/lib) #it is - to find boost!

add_subdirectory(DVSEmulator)
add_subdirectory(LightSpotPassive)
