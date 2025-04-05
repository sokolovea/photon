cmake_minimum_required(VERSION 3.5)

project(3_bmp_kernel LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(QMAKE_CXXFLAGS_DEBUG ON)
set(QMAKE_LFLAGS_DEBUG ON)

add_executable(3_bmp_kernel main.cpp
    bitmap.h
    imagerowsringbuffer.h
    kernel.h   )

include(GNUInstallDirs)
install(TARGETS 3_bmp_kernel
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
