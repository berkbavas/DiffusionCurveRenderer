cmake_minimum_required(VERSION 3.25.1)

project(DiffusionCurveRenderer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_PREFIX_PATH "$ENV{Qt6_Dir}")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(EXTERNAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/External")
set(QT_IMGUI_DIR "${EXTERNAL_DIR}/qtimgui")
set(EIGEN_DIR "${EXTERNAL_DIR}/Eigen")
set(OPENCV_DIR "${EXTERNAL_DIR}/opencv-4.6.0")

set(INCLUDE_DIR
    "${QT_IMGUI_DIR}/include"
    "${EIGEN_DIR}/include"
    "${OPENCV_DIR}/include"
)
set(LIBS_DIR
    "${QT_IMGUI_DIR}/lib"
    "${OPENCV_DIR}/lib"
)

set(LIBS
    debug imguid
    debug qt_imgui_widgetsd
    optimized imgui
    optimized qt_imgui_widgets
    optimized opencv_core460
    optimized opencv_imgproc460
    optimized opencv_highgui460
    optimized opencv_imgcodecs460
)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets OpenGL Gui Concurrent Xml)

file(GLOB_RECURSE SOURCES Source/*.cpp DiffusionCurveRenderer.qrc)

add_executable(DiffusionCurveRenderer ${SOURCES})

target_include_directories(DiffusionCurveRenderer PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Source" ${INCLUDE_DIR})

target_link_directories(DiffusionCurveRenderer PRIVATE ${LIBS_DIR})

target_link_libraries(DiffusionCurveRenderer Qt6::Core Qt6::Widgets Qt6::OpenGL Qt6::Concurrent Qt6::Xml ${LIBS})

add_custom_command(TARGET DiffusionCurveRenderer
    POST_BUILD COMMAND ${CMAKE_COMMAND}
    -E copy_directory
    "${CMAKE_SOURCE_DIR}/Resources/"
    "$<TARGET_FILE_DIR:DiffusionCurveRenderer>/Resources/"
)

add_custom_command(TARGET DiffusionCurveRenderer
    POST_BUILD COMMAND ${CMAKE_COMMAND}
    -E copy_directory
    "${CMAKE_SOURCE_DIR}/External/opencv-4.6.0/bin/"
    "$<TARGET_FILE_DIR:DiffusionCurveRenderer>"
)

add_custom_command(TARGET DiffusionCurveRenderer
    POST_BUILD COMMAND
    Qt6::windeployqt
    --dir "$<TARGET_FILE_DIR:DiffusionCurveRenderer>"
    "$<TARGET_FILE_DIR:DiffusionCurveRenderer>/$<TARGET_FILE_NAME:DiffusionCurveRenderer>"
)
