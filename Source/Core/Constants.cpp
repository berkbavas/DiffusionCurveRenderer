
#include "Constants.h"

namespace DiffusionCurveRenderer
{

    extern const std::string CONTOUR_RENDERER = "ContourRenderer";
    extern const std::string COLOR_RENDERER = "ColorRenderer";
    extern const std::string DOWNSAMPLE_RENDERER = "DownsampleRenderer";
    extern const std::string UPSAMPLE_RENDERER = "UpsampleRenderer";
    extern const std::string BLUR_RENDERER = "BlurRenderer";
    extern const std::string CURVE_SELECTION_RENDERER = "CurveSelectionRenderer";
    extern const std::string RENDERER_MANAGER = "RendererManager";
    extern const std::string CURVE_CONTAINER_GET_CURVE_AROUND = "CurveContainer::GetCurveAround";
    extern const std::string BEZIER_FIND_COLOR_POINT_AROUND = "Bezier::FindColorPointAround";

    extern const std::vector<std::string> ALL_CHORONOMETER_IDs = {
        CONTOUR_RENDERER,
        COLOR_RENDERER,
        DOWNSAMPLE_RENDERER,
        UPSAMPLE_RENDERER,
        BLUR_RENDERER,
        CURVE_SELECTION_RENDERER,
        RENDERER_MANAGER,
        CURVE_CONTAINER_GET_CURVE_AROUND,
        BEZIER_FIND_COLOR_POINT_AROUND
    };

    extern QVector4D USE_THIS_COLOR_WHEN_A_CURVE_SELECTED = QVector4D(0.1, 0.1, 0.1, 1);
}