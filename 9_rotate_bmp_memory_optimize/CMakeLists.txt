cmake_minimum_required(VERSION 3.16)

project(8_rotate_bmp_memory_optimize LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(8_rotate_bmp_memory_optimize main.cpp
    bitmap.h
    bitmapmatrix.h
    rotatematrix.h
    imagenecessaryinfo.h
    pixeltraits.h
    weightscachesingleton.h  )

include(GNUInstallDirs)
install(TARGETS 8_rotate_bmp_memory_optimize
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Optionally add optimization flags
set(CMAKE_CXX_FLAGS_DEBUG "-O2 -g")

# Optionally add optimization flags
set(CMAKE_CXX_FLAGS_RELEASE "-O2")
