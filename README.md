# Diffusion Curve Renderer

A C++ project for reconstructing images using Bézier curves and color diffusion constrained by those curves.

Inspired by the work of [Orzan et al.](https://hal.archives-ouvertes.fr/inria-00274768/). Vectorization algorithms are adapted from [this repository](https://github.com/zhuethanca/DiffusionCurves).

## Features

- Image reconstruction with Bézier curves
- Color diffusion along vectorized curves
- Vectorization algorithms for curve extraction
- Modern GUI with ImGui and Qt
- Uses OpenGL for rendering

## Dependencies

All dependencies are included in the `Libs` folder:

- **ImGui**: For the graphical user interface
- **OpenCV**: For edge detection (built with MSVC 2019 64-bit)
- **Eigen**: For vectorization and linear algebra

## Build Instructions

1. Install **CMake 3.25.1** or newer.
2. Install **Visual Studio 2022** with the **MSVC C++ v143 Compiler**.
3. Install **Qt 6.7.3 MSVC2022 64bit**.
4. Set the environment variable `Qt6_DIR` to `C:\Qt\6.7.3\msvc2022_64`.
5. Clone the repository:
	```
	git clone https://github.com/berkbavas/DiffusionCurveRenderer.git
	```
6. Create and enter the build directory:
	```
	mkdir Build
	cd Build
	```
7. Run CMake:
	```
	cmake ..
	```
8. Open `DiffusionCurveRenderer.sln` in Visual Studio 2022.
9. Build and run the project in **Release** configuration.

## Demo Videos

- [Video 1](https://github.com/user-attachments/assets/a9733a6d-730e-43b0-b889-2ae0fbe6b1fd)
- [Video 2](https://github.com/user-attachments/assets/b4e9dbfe-705d-495c-bcb8-69484ec16fe6)
