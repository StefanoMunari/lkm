cmake_minimum_required(VERSION 3.30)
project(lkm C)
set(LKM_PROJECT_NAME "${CMAKE_PROJECT_NAME}")
#=============================
# Requirements
#=============================
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# default overridable variables (pass -D<NAME_VAR>=VALUE in CMAKE profile)
set(LKM_CLANG_TIDY "ON" CACHE BOOL "")
set(LKM_CMEM_SANITIZER "ON" CACHE BOOL "")
#=============================
# Tools
#=============================
include(cmake/tools/clang_sanitizer.cmake)
include(cmake/tools/clang_tidy.cmake)
include(cmake/tools/kernel.cmake)
#=============================
# Modules
#=============================
include(event/CMakeLists.txt)
include(mmap_phys/CMakeLists.txt)
