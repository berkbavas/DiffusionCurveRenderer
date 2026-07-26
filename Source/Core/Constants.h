#pragma once

#include <string>
#include <vector>

#include <QVector4D>

namespace DiffusionCurveRenderer
{
    // Window
    constexpr int INITIAL_WIDTH = 1600;
    constexpr int INITIAL_HEIGHT = 900;

    // Diffusion
    constexpr float DEFAULT_DIFFUSION_GAP = 0.75f;
    constexpr float DEFAULT_DIFFUSION_WIDTH = 0.75f;
    constexpr float DEFAULT_CONTOUR_THICKNESS = 4.0f;
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
}
