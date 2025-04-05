cmake_minimum_required(VERSION 3.16)

project(2_tiff LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(2_tiff main.cpp
    bitmap.h imageutils.h
    tiff.h
)

# target_link_options(2_tiff PRIVATE "-Wl,--stack,20000000")

include(GNUInstallDirs)
install(TARGETS 2_tiff
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

