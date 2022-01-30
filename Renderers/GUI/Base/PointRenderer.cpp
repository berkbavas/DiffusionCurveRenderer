#include "PointRenderer.h"
#include <Helper.h>
#include <QtMath>

PointRenderer::PointRenderer()
    : mShader(nullptr)
    , mTicks(nullptr)
{}

PointRenderer::~PointRenderer()
{
    if (mShader)
        mShader->deleteLater();

    if (mTicks)
        delete mTicks;

    mShader = nullptr;
    mTicks = nullptr;
}

bool PointRenderer::init()
{
    initializeOpenGLFunctions();

    mShader = new QOpenGLShaderProgram;

    if (!mShader->addShaderFromSourceCode(QOpenGLShader::Vertex, Helper::getBytes(":/Resources/Shaders/Point/VertexShader.vert"))
        || !mShader->addShaderFromSourceCode(QOpenGLShader::Geometry, Helper::getBytes(":/Resources/Shaders/Point/GeometryShader.geom"))
        || !mShader->addShaderFromSourceCode(QOpenGLShader::Fragment, Helper::getBytes(":/Resources/Shaders/Point/FragmentShader.frag"))
        || !mShader->link() || !mShader->bind()) {
        qCritical() << mShader->log();
        return false;
    }

    // Uniforms
    mProjectionMatrixLocation = mShader->uniformLocation("projectionMatrix");
    mPointLocation = mShader->uniformLocation("point");
    mInnerColorLocation = mShader->uniformLocation("innerColor");
    mOuterColorLocation = mShader->uniformLocation("outerColor");
    mInnerRadiusLocation = mShader->uniformLocation("innerRadius");
    mOuterRadiusLocation = mShader->uniformLocation("outerRadius");
    mTicksDeltaLocation = mShader->uniformLocation("ticksDelta");

    //Bind locations
    mShader->bindAttributeLocation("vs_Tick", 0);

    // Ticks
    mTicks = new Ticks(0, 2 * static_cast<float>(M_PI), 100);
    mTicks->create();

    return true;
}

void PointRenderer::render(const Parameters &parameters, const QMatrix4x4 &projectionMatrix)
{
    mShader->bind();

    mShader->setUniformValue(mProjectionMatrixLocation, projectionMatrix);
    mShader->setUniformValue(mPointLocation, parameters.point);
    mShader->setUniformValue(mInnerRadiusLocation, parameters.innerRadius);
    mShader->setUniformValue(mOuterRadiusLocation, parameters.outerRadius);
    mShader->setUniformValue(mInnerColorLocation, parameters.innerColor);
    mShader->setUniformValue(mOuterColorLocation, parameters.outerColor);
    mShader->setUniformValue(mTicksDeltaLocation, mTicks->ticksDelta());

    mTicks->bind();
    glDrawArrays(GL_POINTS, 0, mTicks->size());
    mTicks->release();

    mShader->release();
}
