cmake_minimum_required(VERSION 3.16)

project(7_classification_claude_potier LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(7_classification_claude_potier main.cpp
    bitmap.h
    t_matrix.h)

target_link_options(7_classification_claude_potier PRIVATE "-Wl,--stack,20000000")

include(GNUInstallDirs)
install(TARGETS 7_classification_claude_potier
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Optionally add optimization flags
set(CMAKE_CXX_FLAGS_DEBUG "-fopenmp -O1 -g")

# Optionally add optimization flags
set(CMAKE_CXX_FLAGS_RELEASE "-O1")
