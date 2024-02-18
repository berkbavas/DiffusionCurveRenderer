#pragma once

#include <QVector2D>

#define MAX_CONTROL_POINT_COUNT 32

#define DEFAULT_CONTOUR_THICKNESS 3.0
#define DEFAULT_DIFFUSION_WIDTH 3.0
#define DEFAULT_DIFFUSION_GAP 1.0
#define DEFAULT_CONTOUR_COLOR QVector4D(0, 0, 0, 1)
#define DEFAULT_BLUR_STRENGTH 0.25

#define BLUR_POINT_VISUAL_GAP 25.0
#define COLOR_POINT_VISUAL_GAP 5.0

enum class ShaderType { //
    ContourShader,
    ColorShader,
    ScreenShader,
    DownsampleShader,
    UpsampleShader,
    JacobiShader,
    BlurShader,
    LastPassBlurShader,
    BitmapShader
};

enum class ActionMode { //
    Select,
    AddControlPoint,
    AddColorPoint,
    AddBlurPoint,
};

enum class RenderMode { //
    Contour,
    Diffusion,
    ContourAndDiffusion,
};

enum class Action { //
    Select,
    AddControlPoint,
    AddColorPoint,
    AddBlurPoint,
    RemoveCurve,
    RemoveControlPoint,
    RemoveColorPoint,
    RemoveBlurPoint,
    UpdateControlPointPosition,
    UpdateColorPointPosition,
    UpdateBlurPointPosition,
    LoadFromXML,
    LoadFromJSON,
    SaveAsPNG,
    SaveAsJSON,
    LoadVectorizerImage,
    ShowLoadFromXMLDialog,
    ShowLoadFromJSONDialog,
    ShowSaveAsPNGDialog,
    ShowSaveAsJSONDialog,
    ShowLoadImageDialog,
    ClearCanvas
};

// Vectorizer
enum class WorkMode { //
    Vectorization,
    Edit,
};

enum class SubWorkMode { //
    ViewOriginalImage,
    ViewEdges,
    ViewGaussianStack,
    ChooseEdgeStackLevel
};

enum class VectorizationStatus {
    Ready,
    CreatingGaussianStack,
    CreatingEdgeStack,
    TracingEdges,
    CreatingPolylines,
    ConstructingCurves,
    Finished,
};

struct ProgressStatus {
    float progress; // 0,...,1
    float start;    // 0,...,1
    float end;
};