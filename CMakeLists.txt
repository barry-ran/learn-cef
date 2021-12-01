# 参考cef官方推荐的方式组织项目
# https://github.com/chromiumembedded/cef/blob/f6cf7f9ec7de6f868f04f8d680faeb6c53d4d813/CMakeLists.txt.in#L162

#
# Global setup.
#

# For VS2019 and Xcode 12+ support.
cmake_minimum_required(VERSION 3.19)

# Only generate Debug and Release configuration types.
set(CMAKE_CONFIGURATION_TYPES Debug Release)

# Project name.
project(learn-cef VERSION 0.1 LANGUAGES CXX)

# Use folders in the resulting project files.
set_property(GLOBAL PROPERTY OS_FOLDERS ON)


#
# CEF_ROOT setup.
# This variable must be set to locate the binary distribution.
#

set(CEF_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/third_party/cef")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CEF_ROOT}/cmake")


#
# Load the CEF configuration.
#

# Execute FindCEF.cmake which must exist in CMAKE_MODULE_PATH.
find_package(CEF REQUIRED)


#
# Define CEF-based targets.
#

# Include the libcef_dll_wrapper target.
# Comes from the libcef_dll/CMakeLists.txt file in the binary distribution
# directory.
add_subdirectory(${CEF_LIBCEF_DLL_WRAPPER_PATH} libcef_dll_wrapper)

# Include application targets.
# Comes from the <target>/CMakeLists.txt file in the current directory.
add_subdirectory(example/cefsimple)

# 编译Qt相关的需要设置本地Qt路径到环境变量
# 例如mac下 export PATH=$PATH:/Users/bytedance/Qt/5.9.9/clang_64/lib/cmake/Qt5
add_subdirectory(example/QtOSRCef)
add_subdirectory(example/cppjs)

# Display configuration settings.
PRINT_CEF_CONFIG()
