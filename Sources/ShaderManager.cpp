#include "ShaderManager.h"
#include "Shader.h"

DiffusionCurveRenderer::ShaderManager::ShaderManager(QObject* parent)
    : Manager(parent)
{}

bool DiffusionCurveRenderer::ShaderManager::Init()
{
    // Contour Shader
    {
        Shader* shader = new Shader(ShaderType::ContourShader);
        mShaders.insert(shader->GetType(), shader);
        shader->AddPath(QOpenGLShader::Vertex, ":/Resources/Shaders/Contour.vert");
        shader->AddPath(QOpenGLShader::Fragment, ":/Resources/Shaders/Contour.frag");
        shader->AddPath(QOpenGLShader::Geometry, ":/Resources/Shaders/Contour.geom");
        shader->AddUniform("projection");
        shader->AddUniform("color");
        shader->AddUniform("thickness");
        shader->AddUniform("zoom");
        shader->AddUniform("pointDelta");
        shader->AddUniform("controlPoints");
        shader->AddUniform("controlPointsCount");
        shader->AddAttribute("point");

        if (!shader->Init())
            return false;
    }

    // Color Shader
    {
        Shader* shader = new Shader(ShaderType::ColorShader);
        mShaders.insert(shader->GetType(), shader);
        shader->AddPath(QOpenGLShader::Vertex, ":/Resources/Shaders/Color.vert");
        shader->AddPath(QOpenGLShader::Fragment, ":/Resources/Shaders/Color.frag");
        shader->AddPath(QOpenGLShader::Geometry, ":/Resources/Shaders/Color.geom");
        shader->AddUniform("projection");
        shader->AddUniform("pointsDelta");
        shader->AddUniform("diffusionWidth");
        shader->AddUniform("diffusionGap");
        shader->AddUniform("zoom");
        shader->AddUniform("controlPoints");
        shader->AddUniform("controlPointsCount");
        shader->AddUniform("leftColors");
        shader->AddUniform("leftColorPositions");
        shader->AddUniform("leftColorsCount");
        shader->AddUniform("rightColors");
        shader->AddUniform("rightColorPositions");
        shader->AddUniform("rightColorsCount");
        shader->AddUniform("blurPointPositions");
        shader->AddUniform("blurPointStrengths");
        shader->AddUniform("blurPointsCount");
        shader->AddAttribute("point");

        if (!shader->Init())
            return false;
    }

    // Screen Shader
    {
        Shader* shader = new Shader(ShaderType::ScreenShader);
        mShaders.insert(shader->GetType(), shader);
        shader->AddPath(QOpenGLShader::Vertex, ":/Resources/Shaders/Screen.vert");
        shader->AddPath(QOpenGLShader::Fragment, ":/Resources/Shaders/Screen.frag");
        shader->AddUniform("sourceTexture");
        shader->AddUniform("widthRatio");
        shader->AddUniform("heightRatio");
        shader->AddAttribute("position");
        shader->AddAttribute("textureCoords");

        if (!shader->Init())
            return false;
    }

    // Downsample Shader
    {
        Shader* shader = new Shader(ShaderType::DownsampleShader);
        mShaders.insert(shader->GetType(), shader);
        shader->AddPath(QOpenGLShader::Vertex, ":/Resources/Shaders/Downsample.vert");
        shader->AddPath(QOpenGLShader::Fragment, ":/Resources/Shaders/Downsample.frag");
        shader->AddUniform("colorTexture");
        shader->AddUniform("blurTexture");
        shader->AddAttribute("position");
        shader->AddAttribute("textureCoords");

        if (!shader->Init())
            return false;
    }

    // Upsample Shader
    {
        Shader* shader = new Shader(ShaderType::UpsampleShader);
        mShaders.insert(shader->GetType(), shader);
        shader->AddPath(QOpenGLShader::Vertex, ":/Resources/Shaders/Upsample.vert");
        shader->AddPath(QOpenGLShader::Fragment, ":/Resources/Shaders/Upsample.frag");
        shader->AddUniform("colorSourceTexture");
        shader->AddUniform("colorTargetTexture");
        shader->AddUniform("blurSourceTexture");
        shader->AddUniform("blurTargetTexture");

        shader->AddAttribute("position");
        shader->AddAttribute("textureCoords");

        if (!shader->Init())
            return false;
    }

    // Jacobi Shader
    {
        Shader* shader = new Shader(ShaderType::JacobiShader);
        mShaders.insert(shader->GetType(), shader);
        shader->AddPath(QOpenGLShader::Vertex, ":/Resources/Shaders/Jacobi.vert");
        shader->AddPath(QOpenGLShader::Fragment, ":/Resources/Shaders/Jacobi.frag");
        shader->AddUniform("colorConstrainedTexture");
        shader->AddUniform("colorTargetTexture");
        shader->AddUniform("blurConstrainedTexture");
        shader->AddUniform("blurTargetTexture");
        shader->AddAttribute("position");
        shader->AddAttribute("textureCoords");

        if (!shader->Init())
            return false;
    }

    // Blur Shader
    {
        Shader* shader = new Shader(ShaderType::BlurShader);
        mShaders.insert(shader->GetType(), shader);
        shader->AddPath(QOpenGLShader::Vertex, ":/Resources/Shaders/Blur.vert");
        shader->AddPath(QOpenGLShader::Fragment, ":/Resources/Shaders/Blur.frag");
        shader->AddUniform("colorTexture");
        shader->AddUniform("blurTexture");
        shader->AddUniform("widthRatio");
        shader->AddUniform("heightRatio");
        shader->AddAttribute("position");
        shader->AddAttribute("textureCoords");

        if (!shader->Init())
            return false;
    }

    // Last Pass Blur Shader
    {
        Shader* shader = new Shader(ShaderType::LastPassBlurShader);
        mShaders.insert(shader->GetType(), shader);
        shader->AddPath(QOpenGLShader::Vertex, ":/Resources/Shaders/LastPassBlur.vert");
        shader->AddPath(QOpenGLShader::Fragment, ":/Resources/Shaders/LastPassBlur.frag");
        shader->AddPath(QOpenGLShader::Geometry, ":/Resources/Shaders/LastPassBlur.geom");
        shader->AddUniform("projection");
        shader->AddUniform("pointsDelta");
        shader->AddUniform("diffusionWidth");
        shader->AddUniform("zoom");
        shader->AddUniform("controlPoints");
        shader->AddUniform("controlPointsCount");
        shader->AddUniform("blurPointPositions");
        shader->AddUniform("blurPointStrengths");
        shader->AddUniform("blurPointsCount");
        shader->AddAttribute("point");

        if (!shader->Init())
            return false;
    }

    // Bitmap Shader
    {
        Shader* shader = new Shader(ShaderType::BitmapShader);
        mShaders.insert(shader->GetType(), shader);
        shader->AddPath(QOpenGLShader::Vertex, ":/Resources/Shaders/Bitmap.vert");
        shader->AddPath(QOpenGLShader::Fragment, ":/Resources/Shaders/Bitmap.frag");
        shader->AddUniform("projection");
        shader->AddUniform("transformation");
        shader->AddUniform("sourceTexture");
        shader->AddUniform("widthRatio");
        shader->AddUniform("heightRatio");
        shader->AddAttribute("position");
        shader->AddAttribute("textureCoords");

        if (!shader->Init())
            return false;
    }

    return true;
}

bool DiffusionCurveRenderer::ShaderManager::Bind(ShaderType shader)
{
    mActiveShader = shader;
    return mShaders.value(mActiveShader)->Bind();
}

void DiffusionCurveRenderer::ShaderManager::Release()
{
    mShaders.value(mActiveShader)->Release();
}

void DiffusionCurveRenderer::ShaderManager::SetUniformValue(const QString& name, int value)
{
    mShaders.value(mActiveShader)->SetUniformValue(name, value);
}

void DiffusionCurveRenderer::ShaderManager::SetUniformValue(const QString& name, unsigned int value)
{
    mShaders.value(mActiveShader)->SetUniformValue(name, value);
}

void DiffusionCurveRenderer::ShaderManager::SetUniformValue(const QString& name, float value)
{
    mShaders.value(mActiveShader)->SetUniformValue(name, value);
}

void DiffusionCurveRenderer::ShaderManager::SetUniformValue(const QString& name, const QVector3D& value)
{
    mShaders.value(mActiveShader)->SetUniformValue(name, value);
}

void DiffusionCurveRenderer::ShaderManager::SetUniformValue(const QString& name, const QVector4D& value)
{
    mShaders.value(mActiveShader)->SetUniformValue(name, value);
}

void DiffusionCurveRenderer::ShaderManager::SetUniformValue(const QString& name, const QMatrix4x4& value)
{
    mShaders.value(mActiveShader)->SetUniformValue(name, value);
}

void DiffusionCurveRenderer::ShaderManager::SetUniformValue(const QString& name, const QMatrix3x3& value)
{
    mShaders.value(mActiveShader)->SetUniformValue(name, value);
}

void DiffusionCurveRenderer::ShaderManager::SetUniformValueArray(const QString& name, const QVector<float>& values)
{
    mShaders.value(mActiveShader)->SetUniformValueArray(name, values);
}

void DiffusionCurveRenderer::ShaderManager::SetUniformValueArray(const QString& name, const QVector<QVector4D>& values)
{
    mShaders.value(mActiveShader)->SetUniformValueArray(name, values);
}

void DiffusionCurveRenderer::ShaderManager::SetUniformValueArray(const QString& name, const QVector<QVector3D>& values)
{
    mShaders.value(mActiveShader)->SetUniformValueArray(name, values);
}

void DiffusionCurveRenderer::ShaderManager::SetUniformValueArray(const QString& name, const QVector<QVector2D>& values)
{
    mShaders.value(mActiveShader)->SetUniformValueArray(name, values);
}

void DiffusionCurveRenderer::ShaderManager::SetSampler(const QString& name, unsigned int unit, unsigned int id, GLenum target)
{
    mShaders.value(mActiveShader)->SetSampler(name, unit, id, target);
}

DiffusionCurveRenderer::ShaderManager* DiffusionCurveRenderer::ShaderManager::Instance()
{
    static ShaderManager instance;
    return &instance;
}