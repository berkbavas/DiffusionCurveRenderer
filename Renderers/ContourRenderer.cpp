#include "ContourRenderer.h"
#include "Helper.h"
#include <QFile>

ContourRenderer::ContourRenderer()
    : mShader(nullptr)
    , mTicks(nullptr)
    , mContourThickness(1.0f)
    , mContourColor(0, 0, 0, 1)
{}

ContourRenderer::~ContourRenderer()
{
    if (mShader)
        mShader->deleteLater();

    if (mTicks)
        delete mTicks;

    mShader = nullptr;
    mTicks = nullptr;
}

bool ContourRenderer::init()
{
    initializeOpenGLFunctions();
    mShader = new QOpenGLShaderProgram;

    if (!mShader->addShaderFromSourceCode(QOpenGLShader::Vertex, Helper::getBytes(":/Resources/Shaders/Contour/VertexShader.vert"))
        || !mShader->addShaderFromSourceCode(QOpenGLShader::Fragment, Helper::getBytes(":/Resources/Shaders/Contour/FragmentShader.frag"))
        || !mShader->addShaderFromSourceCode(QOpenGLShader::Geometry, Helper::getBytes(":/Resources/Shaders/Contour/GeometryShader.geom"))
        || !mShader->link() || !mShader->bind()) {
        qCritical() << mShader->log();
        return false;
    }

    // Uniforms
    mProjectionMatrixLocation = mShader->uniformLocation("projectionMatrix");
    mThicknessLocation = mShader->uniformLocation("thickness");
    mColorLocation = mShader->uniformLocation("color");
    mTicksDeltaLocation = mShader->uniformLocation("ticksDelta");
    mControlPointsCountLocation = mShader->uniformLocation("controlPointsCount");

    // Locations
    mShader->bindAttributeLocation("vs_Tick", 0);

    // Shader Storage Buffer Object;
    glGenBuffers(1, &mControlPointsBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, mControlPointsBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, mControlPointsBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, Constants::MAX_CONTROL_POINT_COUNT * sizeof(QVector2D), nullptr, GL_DYNAMIC_COPY);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // Ticks
    mTicks = new Ticks(0, 1, 200);
    mTicks->create();

    mShader->release();

    return true;
}

void ContourRenderer::render(const QVector<Curve *> &curves, const QMatrix4x4 &projectionMatrix)
{
    for (int i = 0; i < curves.size(); ++i) {
        Bezier *curve = dynamic_cast<Bezier *>(curves[i]);

        if (curve == nullptr)
            continue;

        if (curve->contourColorEnabled()) {
            mShader->bind();

            // Control Points Buffer
            QVector<QVector2D> controlPoints = curve->getControlPointPositions();
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, mControlPointsBuffer);
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(QVector2D) * controlPoints.size(), controlPoints.constData());
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

            // Uniform Variables
            mShader->setUniformValue(mProjectionMatrixLocation, projectionMatrix);
            mShader->setUniformValue(mColorLocation, mContourColor);
            mShader->setUniformValue(mThicknessLocation, mContourThickness);
            mShader->setUniformValue(mTicksDeltaLocation, mTicks->ticksDelta());
            mShader->setUniformValue(mControlPointsCountLocation, (GLint) controlPoints.size());

            mTicks->bind();
            glDrawArrays(GL_POINTS, 0, mTicks->size());
            mTicks->release();

            mShader->release();
        }
    }
}

QVector4D ContourRenderer::lighter(QVector4D color, float factor)
{
    float x = color.x() + factor * (1 - color.x());
    float y = color.y() + factor * (1 - color.y());
    float z = color.z() + factor * (1 - color.z());
    float w = color.w();

    if (1 < x)
        x = 1;
    if (1 < y)
        y = 1;
    if (1 < z)
        z = 1;

    return QVector4D(x, y, z, w);
}

const QVector4D &ContourRenderer::contourColor() const
{
    return mContourColor;
}

void ContourRenderer::setContourColor(const QVector4D &newContourColor)
{
    mContourColor = newContourColor;
}

float ContourRenderer::contourThickness() const
{
    return mContourThickness;
}

void ContourRenderer::setContourThickness(float newContourThickness)
{
    mContourThickness = newContourThickness;
}
