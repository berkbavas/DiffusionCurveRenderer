#ifndef TYPES_H
#define TYPES_H

typedef int Location;

namespace Constants {
const int MAX_CONTROL_POINT_COUNT = 32;
const int MAX_COLOR_POINT_COUNT = 32;
} // namespace Constants

enum class Action {
    Select,
    AppendControlPoint,
    InsertControlPoint,
    AddColorPoint,
    Move,
    Pan,
    ZoomIn,
    ZoomOut,
    RemoveCurve,
    RemoveControlPoint,
    RemoveColorPoint,
    UpdateControlPointPosition,
    UpdateControlPointXPosition,
    UpdateControlPointYPosition,
    UpdateCurveZIndex,
    UpdateColorPointColor,
    UpdateColorPointPosition,
    UpdateContourThickness,
    UpdateDiffusionWidth,
    UpdateContourColor,
    UpdateSmoothIterations,
    UpdateRenderQuality,
    UpdateMode,
    UpdateRenderMode,
    UpdateColorRendererMode,
    ShowLoadFromJsonDialog,
    ShowLoadFromXmlDialog,
    ShowSaveAsJsonDialog,
    ShowSaveAsPngDialog,
    LoadFromJson,
    LoadFromXml,
    SaveAsJson,
    SaveAsPng,
    ClearCanvas,

};

enum class Mode {
    Select,
    AppendControlPoint,
    InsertControlPoint,
    AddColorPoint,
    MoveCurve,
    Pan,
};

enum class RenderMode { None = 0x01, Contours = 0x02, Diffuse = 0x04 };

struct ProjectionParameters
{
    float left;
    float right;
    float bottom;
    float top;
    float width;
    float height;
    float zoomRatio;
    float pixelRatio;
};

enum class ColorRendererMode { LineStrip, TriangleStrip };

enum class RenderQuality { Low, Fair, High };

RenderMode operator+(const RenderMode &lhs, const RenderMode &rhs);
bool operator&(const RenderMode &lhs, const RenderMode &rhs);

#endif // TYPES_H
