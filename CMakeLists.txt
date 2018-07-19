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


add_executable(CPUThreadPool src/CPU线程池/CPUThreadPool.cpp)
target_link_libraries(CPUThreadPool pthread)

add_executable(IOThreadPool src/IO线程池/IOThreadPool.cpp)