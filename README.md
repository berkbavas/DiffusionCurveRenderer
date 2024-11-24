# Diffusion Curve Renderer

Reconstructing images using Bezier curves and diffusing colors constrained by those curves.

The ideas belong to [Orzan et al.](https://hal.archives-ouvertes.fr/inria-00274768/)

Vectorization algorithms are taken from this [repo](https://github.com/zhuethanca/DiffusionCurves) and adopted.

The project is developed using `OpenGL`, `OpenCV`, `Eigen`, `ImGui` and `Qt` and is written in `C++`.

## Dependencies

This project has 3 dependencies and all of them is under `Libs` folder.
I use `ImGui` for GUI; `OpenCV` for edge detection and `Eigen` for vectorization.
`OpenCV`'s binaries were built with `MSVC 2019 64bit` compiler.

## Build

1) Install `CMake 3.25.1` or above.
2) Install `Visual Studio 2022` and `MSVC C++ v143 Compiler`.
3) Install `Qt 6.7.3 MSVC2022 64bit` kit.
4) Set environment variable `Qt6_DIR` as `C:\Qt\6.7.3\msvc2022_64`.
5) Clone the repo `git clone https://github.com/berkbavas/DiffusionCurveRenderer.git`.
6) Create a folder `mkdir Build`.
7) Enter the folder `cd Build`.
8) Run CMake `cmake ..`.
9) Open `DiffusionCurveRenderer.sln` with `Visual Studio 2022`.
10) Build & Run with `Release` configuration.

## Videos

https://github.com/user-attachments/assets/a9733a6d-730e-43b0-b889-2ae0fbe6b1fd

https://github.com/user-attachments/assets/b4e9dbfe-705d-495c-bcb8-69484ec16fe6
