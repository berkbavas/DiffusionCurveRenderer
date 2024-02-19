cmake_minimum_required(VERSION 3.25.1)

project(DiffusionCurveRenderer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_PREFIX_PATH "$ENV{Qt6_Dir}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(INCLUDE_DIR Dependencies/qtimgui/include/qtimgui
                Dependencies/qtimgui/include/imgui
                Dependencies/qtimgui/include/implot
                Dependencies/Eigen/include
                Dependencies/opencv-4.6.0/include)
                
set(LIBS_DIR Dependencies/qtimgui/lib
             Dependencies/opencv-4.6.0/lib)
             
set(LIBS imgui 
         implot 
         qt_imgui_widgets 
         opencv_core460 
         opencv_imgproc460
         opencv_highgui460
         opencv_imgcodecs460)

find_package(Qt6 COMPONENTS Core Widgets OpenGL Gui Concurrent Xml REQUIRED)

add_compile_options(/bigobj)

file(GLOB SOURCES Sources/*.cpp Sources/*.h *.qrc)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}" ${INCLUDE_DIR})

link_directories(${LIBS_DIR})

add_executable(DiffusionCurveRenderer ${SOURCES})

target_link_libraries(DiffusionCurveRenderer Qt6::Core Qt6::Widgets Qt6::OpenGL Qt6::Concurrent Qt6::Xml ${LIBS})

add_custom_command(TARGET DiffusionCurveRenderer
    POST_BUILD COMMAND xcopy /E /Y 
    "\"$(SolutionDir)../Dependencies/opencv-4.6.0/bin\""
    "\"$(OutDir)\""
)

add_custom_command(TARGET DiffusionCurveRenderer
    POST_BUILD COMMAND "$ENV{Qt6_Dir}/bin/windeployqt.exe"
    "\"$(OutDir)$(TargetName)$(TargetExt)\""
)