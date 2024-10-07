#pragma once

#include <QVector4D>
#include <string>
#include <vector>

namespace DiffusionCurveRenderer
{
    // Window
    constexpr int INITIAL_WIDTH = 1600;
    constexpr int INITIAL_HEIGHT = 900;

    // Diffusion
    constexpr float DEFAULT_DIFFUSION_GAP = 1.0f;
    constexpr float DEFAULT_DIFFUSION_WIDTH = 4.0f;
    constexpr float DEFAULT_CONTOUR_THICKNESS = 16.0f;
    constexpr float DEFAULT_BLUR_STRENGTH = 0.25f;
    constexpr int DEFAULT_SMOOTH_ITERATIONS = 20;

    // General render settings
    constexpr int NUMBER_OF_INTERVALS = 100;
    constexpr int DEFAULT_FRAMEBUFFER_SIZE = 2048;

    // Curve selection (Obsolete)
    constexpr float DEFAULT_CURVE_SELECTION_WIDTH = 20.0f;

    // Overlay painter (Gui)
    constexpr float HANDLE_OUTER_DISK_RADIUS_PX = 15.0f; // Pixels
    constexpr float HANDLE_INNER_DISK_RADIUS_PX = 8.0f;  // Pixels
    constexpr float COLOR_POINT_HANDLE_OFFSET_PX = 0.0f; // Pixels

    // Others
    extern QVector4D USE_THIS_COLOR_WHEN_A_CURVE_SELECTED;

    // Choronometer IDs
    extern const std::string CONTOUR_RENDERER;
    extern const std::string COLOR_RENDERER;
    extern const std::string DOWNSAMPLE_RENDERER;
    extern const std::string UPSAMPLE_RENDERER;
    extern const std::string BLUR_RENDERER;
    extern const std::string CURVE_SELECTION_RENDERER;
    extern const std::string RENDERER_MANAGER;
    extern const std::string CURVE_CONTAINER_GET_CURVE_AROUND;
    extern const std::string BEZIER_FIND_COLOR_POINT_AROUND;

    extern const std::vector<std::string> ALL_CHORONOMETER_IDs;
}
