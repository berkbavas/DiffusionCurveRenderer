#include "ColorRenderer.h"

#include <Curves/Bezier.h>

#include <Helper.h>

ColorRenderer::ColorRenderer()
    : mLineStripShader(nullptr)
    , mTriangleStripShader(nullptr)
    , mTicks(nullptr)
    , mInit(false)
{}

ColorRenderer::~ColorRenderer()
{
    if (mLineStripShader)
        delete mLineStripShader;
    if (mTriangleStripShader)
        delete mTriangleStripShader;
    if (mTicks)
        delete mTicks;

    mLineStripShader = nullptr;
    mTriangleStripShader = nullptr;
    mTicks = nullptr;
}

bool ColorRenderer::init()
{
    initializeOpenGLFunctions();

    // Line strip shader
    {
        mLineStripShader = new QOpenGLShaderProgram;
        if (!mLineStripShader->addShaderFromSourceCode(QOpenGLShader::Vertex,
                                                       Helper::getBytes(":/Resources/Shaders/Diffusion/Color/LineStrip/VertexShader.vert"))
            || !mLineStripShader->addShaderFromSourceCode(QOpenGLShader::Fragment,
                                                          Helper::getBytes(":/Resources/Shaders/Diffusion/Color/LineStrip/FragmentShader.frag"))
            || !mLineStripShader->addShaderFromSourceCode(QOpenGLShader::Geometry,
                                                          Helper::getBytes(":/Resources/Shaders/Diffusion/Color/LineStrip/GeometryShader.geom"))
            || !mLineStripShader->link() || !mLineStripShader->bind()) {
            qCritical() << mLineStripShader->log();
            return false;
        }

        // Uniform Variables

        mLineStripLocations.insert("projectionMatrix", mLineStripShader->uniformLocation("projectionMatrix"));
        mLineStripLocations.insert("ticksDelta", mLineStripShader->uniformLocation("ticksDelta"));
        mLineStripLocations.insert("gap", mLineStripShader->uniformLocation("gap"));

        mLineStripLocations.insert("controlPoints", mLineStripShader->uniformLocation("controlPoints"));
        mLineStripLocations.insert("controlPointsCount", mLineStripShader->uniformLocation("controlPointsCount"));

        mLineStripLocations.insert("leftColors", mLineStripShader->uniformLocation("leftColors"));
        mLineStripLocations.insert("leftColorPositions", mLineStripShader->uniformLocation("leftColorPositions"));
        mLineStripLocations.insert("leftColorsCount", mLineStripShader->uniformLocation("leftColorsCount"));

        mLineStripLocations.insert("rightColors", mLineStripShader->uniformLocation("rightColors"));
        mLineStripLocations.insert("rightColorPositions", mLineStripShader->uniformLocation("rightColorPositions"));
        mLineStripLocations.insert("rightColorsCount", mLineStripShader->uniformLocation("rightColorsCount"));

        qDebug().noquote() << "ColorRenderer Line Strip Locations:" << mLineStripLocations;

        // Attribute Locations
        mLineStripShader->bindAttributeLocation("vs_Tick", 0);

        mLineStripShader->release();
    }

    // Triangle strip shader
    {
        mTriangleStripShader = new QOpenGLShaderProgram;
        if (!mTriangleStripShader->addShaderFromSourceCode(QOpenGLShader::Vertex,
                                                           Helper::getBytes(":/Resources/Shaders/Diffusion/Color/TriangleStrip/VertexShader.vert"))
            || !mTriangleStripShader->addShaderFromSourceCode(QOpenGLShader::Fragment,
                                                              Helper::getBytes(":/Resources/Shaders/Diffusion/Color/TriangleStrip/FragmentShader.frag"))
            || !mTriangleStripShader->addShaderFromSourceCode(QOpenGLShader::Geometry,
                                                              Helper::getBytes(":/Resources/Shaders/Diffusion/Color/TriangleStrip/GeometryShader.geom"))
            || !mTriangleStripShader->link() || !mTriangleStripShader->bind()) {
            qCritical() << mTriangleStripShader->log();
            return false;
        }

        // Uniform Variables

        mTriangleStripLocations.insert("projectionMatrix", mTriangleStripShader->uniformLocation("projectionMatrix"));
        mTriangleStripLocations.insert("ticksDelta", mTriangleStripShader->uniformLocation("ticksDelta"));
        mTriangleStripLocations.insert("width", mTriangleStripShader->uniformLocation("width"));

        mTriangleStripLocations.insert("controlPoints", mTriangleStripShader->uniformLocation("controlPoints"));
        mTriangleStripLocations.insert("controlPointsCount", mTriangleStripShader->uniformLocation("controlPointsCount"));

        mTriangleStripLocations.insert("leftColors", mTriangleStripShader->uniformLocation("leftColors"));
        mTriangleStripLocations.insert("leftColorPositions", mTriangleStripShader->uniformLocation("leftColorPositions"));
        mTriangleStripLocations.insert("leftColorsCount", mTriangleStripShader->uniformLocation("leftColorsCount"));

        mTriangleStripLocations.insert("rightColors", mTriangleStripShader->uniformLocation("rightColors"));
        mTriangleStripLocations.insert("rightColorPositions", mTriangleStripShader->uniformLocation("rightColorPositions"));
        mTriangleStripLocations.insert("rightColorsCount", mTriangleStripShader->uniformLocation("rightColorsCount"));

        qDebug().noquote() << "ColorRenderer Triangle Strip Locations:" << mTriangleStripLocations;

        // Attribute Locations
        mTriangleStripShader->bindAttributeLocation("vs_Tick", 0);

        mTriangleStripShader->release();
    }

    mTicks = new Ticks(0, 1.0, 100);
    mTicks->create();

    return mInit = true;
}

void ColorRenderer::render(const Parameters &parameters)
{
    if (!mInit)
        return;

    if (parameters.mode == ColorRendererMode::LineStrip) {
        mLineStripShader->bind();
        mTicks->bind();

        mLineStripShader->setUniformValue(mLineStripLocations.value("projectionMatrix"), parameters.projectionMatrix);
        mLineStripShader->setUniformValue(mLineStripLocations.value("ticksDelta"), mTicks->ticksDelta());
        mLineStripShader->setUniformValue(mLineStripLocations.value("gap"), parameters.gap);

        for (int i = 0; i < parameters.curves.size(); ++i) {
            Bezier *curve = dynamic_cast<Bezier *>(parameters.curves[i]);

            if (curve == nullptr)
                continue;

            curve->scale(parameters.quality);

            // Control points
            QVector<QVector2D> controlPoints = curve->getControlPointPositions();
            mLineStripShader->setUniformValueArray(mLineStripLocations.value("controlPoints"), controlPoints.constData(), controlPoints.size());
            mLineStripShader->setUniformValue(mLineStripLocations.value("controlPointsCount"), (GLint) controlPoints.size());

            // Left colors
            QVector<QVector4D> leftColors = curve->getLeftColors();
            QVector<float> leftColorPositions = curve->getLeftColorPositions();

            mLineStripShader->setUniformValueArray(mLineStripLocations.value("leftColors"), leftColors.constData(), leftColors.size());
            mLineStripShader->setUniformValueArray(mLineStripLocations.value("leftColorPositions"),
                                                   leftColorPositions.constData(),
                                                   leftColorPositions.size(),
                                                   1);
            mLineStripShader->setUniformValue(mLineStripLocations.value("leftColorsCount"), (GLint) leftColors.size());

            // Right colors
            QVector<QVector4D> rightColors = curve->getRightColors();
            QVector<float> rightColorPositions = curve->getRightColorPositions();
            mLineStripShader->setUniformValueArray(mLineStripLocations.value("rightColors"), rightColors.constData(), rightColors.size());
            mLineStripShader->setUniformValueArray(mLineStripLocations.value("rightColorPositions"),
                                                   rightColorPositions.constData(),
                                                   rightColorPositions.size(),
                                                   1);
            mLineStripShader->setUniformValue(mLineStripLocations.value("rightColorsCount"), (GLint) rightColors.size());

            glDrawArrays(GL_POINTS, 0, mTicks->size());

            curve->scale(1.0f / parameters.quality);
        }

        mTicks->release();
        mLineStripShader->release();
    }

    if (parameters.mode == ColorRendererMode::TriangleStrip) {
        mTriangleStripShader->bind();
        mTicks->bind();

        mTriangleStripShader->setUniformValue(mTriangleStripLocations.value("projectionMatrix"), parameters.projectionMatrix);
        mTriangleStripShader->setUniformValue(mTriangleStripLocations.value("ticksDelta"), mTicks->ticksDelta());
        mTriangleStripShader->setUniformValue(mTriangleStripLocations.value("width"), parameters.width);

        for (int i = 0; i < parameters.curves.size(); ++i) {
            Bezier *curve = dynamic_cast<Bezier *>(parameters.curves[i]);

            if (curve == nullptr)
                continue;

            curve->scale(parameters.quality);

            // Control points
            QVector<QVector2D> controlPoints = curve->getControlPointPositions();
            mTriangleStripShader->setUniformValueArray(mTriangleStripLocations.value("controlPoints"),
                                                       controlPoints.constData(),
                                                       controlPoints.size());
            mTriangleStripShader->setUniformValue(mTriangleStripLocations.value("controlPointsCount"), (GLint) controlPoints.size());

            // Left colors
            QVector<QVector4D> leftColors = curve->getLeftColors();
            QVector<float> leftColorPositions = curve->getLeftColorPositions();

            mTriangleStripShader->setUniformValueArray(mTriangleStripLocations.value("leftColors"), leftColors.constData(), leftColors.size());
            mTriangleStripShader->setUniformValueArray(mTriangleStripLocations.value("leftColorPositions"),
                                                       leftColorPositions.constData(),
                                                       leftColorPositions.size(),
                                                       1);
            mTriangleStripShader->setUniformValue(mTriangleStripLocations.value("leftColorsCount"), (GLint) leftColors.size());

            // Right colors
            QVector<QVector4D> rightColors = curve->getRightColors();
            QVector<float> rightColorPositions = curve->getRightColorPositions();
            mTriangleStripShader->setUniformValueArray(mTriangleStripLocations.value("rightColors"), rightColors.constData(), rightColors.size());
            mTriangleStripShader->setUniformValueArray(mTriangleStripLocations.value("rightColorPositions"),
                                                       rightColorPositions.constData(),
                                                       rightColorPositions.size(),
                                                       1);
            mTriangleStripShader->setUniformValue(mTriangleStripLocations.value("rightColorsCount"), (GLint) rightColors.size());

            glDrawArrays(GL_POINTS, 0, mTicks->size());

            curve->scale(1.0f / parameters.quality);
        }

        mTicks->release();
        mTriangleStripShader->release();
    }
}
