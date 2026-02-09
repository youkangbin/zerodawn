# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(openglwindow LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/debug")
# 可选：设置 Release 版本 exe 输出到 构建目录/release（推荐，保持结构统一）
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/release")

set(Qt6_DIR "C:/Qt/6.5.3/msvc2019_64/lib/cmake/Qt6")

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/opengl/openglwindow")

list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.5.3/msvc2019_64")
find_package(Qt6 REQUIRED COMPONENTS 
Core Gui OpenGL Widgets OpenGLWidgets)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

qt_standard_project_setup()

qt_add_executable(openglwindow
    main.cpp
    TrailWidget.cpp 
    TrailWidget.hpp
    MainWindow.hpp
    MainWindow.cpp
    resources.qrc
)

set_target_properties(openglwindow PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(openglwindow PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::OpenGL
    Qt6::Widgets        # 新增
    Qt6::OpenGLWidgets  # 新增
)
