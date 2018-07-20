cmake_minimum_required(VERSION 3.10)
project(ThreadPool)

# 添加c++ 14标准支持
set( CMAKE_CXX_FLAGS "-std=gnu++14" )

# 添加cmake_modules
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake_modules)

# 添加库文件输出路径
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# 添加可执行文件输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

find_package(folly CONFIG)
if (NOT folly_FOUND)
    # Look for folly using our FindFolly.cmake module.  This may work if the
    # folly was built with its older autoconf build files rather than with CMake.
    find_package(Folly MODULE REQUIRED)
endif()

message(folly-include: ${FOLLY_INCLUDE_DIR})
message(folly-library: ${FOLLY_LIBRARIES})

include_directories(${FOLLY_INCLUDE_DIR})

add_executable(CPUThreadPool src/CPU线程池/CPUThreadPool.cpp)
target_link_libraries(CPUThreadPool pthread)

add_executable(CPUThreadPool_NoBlock src/CPU线程池/CPUThreadPool_NoBlock.cpp)
target_link_libraries(CPUThreadPool_NoBlock ${FOLLY_LIBRARIES})

add_executable(IOThreadPool src/IO线程池/IOThreadPool.cpp)