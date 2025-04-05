cmake_minimum_required(VERSION 3.5)

project(5_bmp_quick_average LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(QMAKE_CXXFLAGS_DEBUG ON)
set(QMAKE_LFLAGS_DEBUG ON)

add_executable(5_bmp_quick_average main.cpp
    bitmap.h
    imagerowsringbuffer.h
)

include(GNUInstallDirs)
install(TARGETS 5_bmp_quick_average
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
