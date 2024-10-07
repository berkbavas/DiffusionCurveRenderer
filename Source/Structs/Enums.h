#pragma once

#include <QObject>

namespace DiffusionCurveRenderer
{
    enum class WorkMode
    {
        Vectorization = 0x00,
        CurveEditing = 0x01,
    };

    enum class VectorizationStage
    {
        Initial,
        GaussianStack,
        EdgeStack,
        EdgeTracer,
        Potrace,
        CurveContructor,
        ColorSampler,
        Finished
    };

    enum class VectorizationViewOption
    {
        ViewOriginalImage,
        ViewEdges,
        ViewGaussianStack,
        ChooseEdgeStackLevel
    };

    enum class RenderMode : uint32_t
    {
        Contour = 0x01,
        Diffusion = 0x02
    };

    enum class ColorPointType
    {
        Left,
        Right
    };

    Q_DECLARE_FLAGS(RenderModes, RenderMode);
}

Q_DECLARE_OPERATORS_FOR_FLAGS(DiffusionCurveRenderer::RenderModes);