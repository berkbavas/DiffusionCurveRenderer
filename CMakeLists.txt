cmake_minimum_required(VERSION 3.25.1)

project(DiffusionCurveRenderer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_PREFIX_PATH "$ENV{Qt6_Dir}")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(INCLUDE_DIR 	Libs/qtimgui/include
					Libs/Eigen/include
                    Libs/opencv-4.6.0/include)

set(LIBS_DIR        Libs/qtimgui/lib
                    Libs/opencv-4.6.0/lib)

set(LIBS    debug   debug/imgui  
            debug   debug/qt_imgui_widgets                
            optimized release/imgui
            optimized release/qt_imgui_widgets
            optimized opencv_core460 
            optimized opencv_imgproc460
            optimized opencv_highgui460
            optimized opencv_imgcodecs460)

find_package(Qt6 COMPONENTS Core Widgets OpenGL Gui Concurrent Xml REQUIRED)

add_compile_options(/bigobj)

file(GLOB_RECURSE SOURCES Source/*.cpp Source/*.h *.qrc)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/Source" ${INCLUDE_DIR})

link_directories(${LIBS_DIR})

add_executable(DiffusionCurveRenderer ${SOURCES})

target_link_libraries(DiffusionCurveRenderer Qt6::Core Qt6::Widgets Qt6::OpenGL Qt6::Concurrent Qt6::Xml ${LIBS})

add_custom_command(TARGET DiffusionCurveRenderer 
POST_BUILD COMMAND xcopy /E /Y "\"$(SolutionDir)../Resources\"" "\"$(OutDir)Resources/*\"")
    
add_custom_command(TARGET DiffusionCurveRenderer
POST_BUILD COMMAND xcopy /E /Y  "\"$(SolutionDir)../Resources\"" "\"$(BuildDir)Resources/*\"")
    
add_custom_command(TARGET DiffusionCurveRenderer
 POST_BUILD COMMAND xcopy /E /Y  "\"$(SolutionDir)../Libs/opencv-4.6.0/bin\"" "\"$(OutDir)\"")

add_custom_command(TARGET DiffusionCurveRenderer
POST_BUILD COMMAND "$ENV{Qt6_Dir}/bin/windeployqt.exe" "\"$(OutDir)$(TargetName)$(TargetExt)\"")