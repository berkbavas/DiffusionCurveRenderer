# Diffusion Curve Renderer
Reconstructing images using Bezier curves and diffusing colors constrained by those curves.

If you do not want to import an image to the application in order to run the vectorization pipeline, you can use edit mode to create artistic images from scratch.

The ideas belong to [Orzan et al.](https://hal.archives-ouvertes.fr/inria-00274768/)

I took vectorization algorithms from this [repo](https://github.com/zhuethanca/DiffusionCurves) and adopted.

Vectorization algorithms run on CPU but the diffusion algorithm runs on GPU providing real time editing.

The project is developed using OpenGL, OpenCV, Eigen, ImGui and Qt framework and is written in C++.

## Dependencies
This project has 3 dependencies and all of them is under `Dependencies` folder.
I use ImGui for GUI; OpenCV for edge detection and Eigen for vectorization.
OpenCV's binaries were built with `MSVC 2019 64bit` compiler.

## Build
1) Install `CMake 3.25.1` or latest.
2) Install `Visual Studio 2019` and `MSVC C++ Compiler`.
3) Install `Qt 6.x.x MSVC2019 64bit` kit.
4) Define environment variable `Qt6_DIR` as `C:\Qt\6.x.x\msvc2019_64`.
5) Clone the repo `git clone https://github.com/berkbavas/DiffusionCurveRenderer.git`.
6) Create a folder `mkdir Build`.
7) Enter the folder `cd Build`.
8) Run CMake `cmake ..`.
9) Open `DiffusionCurveRenderer.sln` with `Visual Studio 2019`.
10) Build & Run with `Release` configuration.

## Screenshots
![](Screenshots/0.png)

![](Screenshots/1.png)

![](Screenshots/2.png)

![](Screenshots/3.png)
