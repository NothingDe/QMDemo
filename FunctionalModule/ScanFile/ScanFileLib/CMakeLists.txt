cmake_minimum_required(VERSION 3.10)
project(ScanFile)
set(CMAKE_CXX_STANDARD 11) # 设置C++11
set(CMAKE_CXX_STANDARD_REQUIRED ON) 

include_directories(./) # 添加头文件目录
# debug生成名称
set(CMAKE_DEBUG_POSTFIX "d")
set(SOURCES ScanFile.cpp) 
add_library(ScanFile SHARED ${SOURCES}) # 生成动态库

# 设置导出宏
target_compile_definitions(ScanFile PRIVATE SCANFILE_EXPORTS)

# 安装库
install(TARGETS ScanFile DESTINATION ${CMAKE_BINARY_DIR}/../lib)
