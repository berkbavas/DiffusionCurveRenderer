#ifndef RENDERERMANAGER_H
#define RENDERERMANAGER_H

#include "ContourRenderer.h"
#include "ScreenRenderer.h"

#include <CurveContainer.h>
#include <Renderers/Diffusion/ColorRenderer.h>
#include <Renderers/Diffusion/DownsampleRenderer.h>
#include <Renderers/Diffusion/JacobiRenderer.h>
#include <Renderers/Diffusion/UpsampleRenderer.h>

#include <QOpenGLFramebufferObject>

class RendererManager : protected QOpenGLFunctions
{
public:
    RendererManager();
    ~RendererManager();

    bool init();
    void render();

    void createFramebuffers();
    void deleteFramebuffers();

    void setCurveContainer(const CurveContainer *newCurveContainer);
    void setProjectionParameters(const ProjectionParameters *newProjectionParameters);

public slots:
    void onSmoothIterationsChanged(int smoothIterations);
    void onDiffusionWidthChanged(float diffusionWidth);
    void onContourThicknessChanged(float thickness);
    void onContourColorChanged(const QVector4D &color);
    void onRenderQualityChanged(RenderQuality renderQuality);
    void onRenderModeChanged(RenderMode mode);
    void onColorRendererModeChanged(ColorRendererMode mode);

private slots:
    void diffuse();
    void contours();

private:
    ContourRenderer *mContourRenderer;
    ColorRenderer *mColorRenderer;
    DownsampleRenderer *mDownsampleRenderer;
    UpsampleRenderer *mUpsampleRenderer;
    JacobiRenderer *mJacobiRenderer;
    ScreenRenderer *mScreenRenderer;

    const CurveContainer *mCurveContainer;
    const ProjectionParameters *mProjectionParameters;

    QOpenGLFramebufferObjectFormat mFrambufferFormat;
    QOpenGLFramebufferObject *mInitialFrameBuffer;
    QVector<QOpenGLFramebufferObject *> mTemporaryFrameBuffers;
    QVector<QOpenGLFramebufferObject *> mDownsampledFramebuffers;
    QVector<QOpenGLFramebufferObject *> mUpsampledFramebuffers;
    QOpenGLFramebufferObject *mSaverFrameBuffer;

    RenderMode mRenderMode;
    ColorRendererMode mColorRendererMode;
    RenderQuality mRenderQuality;

    int mBufferSize;
    int mQuality;
    bool mInit;
    bool mRenderQualityChanged;
    float mGap;
    float mDiffusionWidth;
    int mSmoothIterations;
};

#endif // RENDERERMANAGER_H
