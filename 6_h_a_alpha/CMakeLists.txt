cmake_minimum_required(VERSION 3.5)

project(6_h_a_alpha LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Добавление исполняемого файла
add_executable(6_h_a_alpha
    main.cpp
    bitmap.h
    imageutils.h
    tiff.h
    tiffimagereader.h
    imagerowsringbuffer.h
    pixel.h
    t_matrix.h
)

# Подключение заголовочных файлов из текущей директории
target_include_directories(6_h_a_alpha PRIVATE
    ${CMAKE_SOURCE_DIR}
)


# Путь установки
include(GNUInstallDirs)

install(TARGETS 6_h_a_alpha
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES
    bitmap.h
    imageutils.h
    tiff.h
    tiffimagereader.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/6_h_a_alpha
    FILES_MATCHING PATTERN "*.cpp"
)

# Флаги для отладки и релиза
set(CMAKE_CXX_FLAGS_DEBUG "-O1 -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
