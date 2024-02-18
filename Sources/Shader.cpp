#include "Shader.h"
#include "Helper.h"

#include <QDebug>

DiffusionCurveRenderer::Shader::Shader(ShaderType type, QObject* parent)
    : QObject(parent)
    , mType(type)
{
    mShaderName = GetShaderTypeString();
}

bool DiffusionCurveRenderer::Shader::Init()
{
    initializeOpenGLFunctions();

    qInfo() << mShaderName << "is initializing... ";

    mProgram = new QOpenGLShaderProgram;

    auto types = mPaths.keys();

    for (auto type : qAsConst(types))
    {
        if (!mProgram->addShaderFromSourceCode(type, Helper::GetBytes(mPaths[type])))
        {
            qWarning() << Q_FUNC_INFO << "Could not load" << GetShaderTypeString(type);
            mProgram->deleteLater();
            return false;
        }
    }

    if (!mProgram->link())
    {
        qWarning() << Q_FUNC_INFO << "Could not link shader program.";
        mProgram->deleteLater();
        return false;
    }

    if (!mProgram->bind())
    {
        qWarning() << Q_FUNC_INFO << "Could not bind shader program.";
        mProgram->deleteLater();
        return false;
    }

    for (const auto& uniform : qAsConst(mUniforms))
    {
        mLocations.insert(uniform, mProgram->uniformLocation(uniform));
    }

    auto uniformArrays = mUniformArrays.keys();
    for (const auto& uniformArray : qAsConst(uniformArrays))
    {
        int size = mUniformArrays[uniformArray];

        for (int i = 0; i < size; ++i)
        {
            QString name = uniformArray.arg(QString::number(i));
            mLocations.insert(name, mProgram->uniformLocation(name));
        }
    }

    for (int i = 0; i < mAttributes.size(); ++i)
    {
        mProgram->bindAttributeLocation(mAttributes[i], i);
    }

    mProgram->release();

    qInfo() << mShaderName << "is initialized.";
    qInfo() << "Uniform locations are:" << mLocations;

    return true;
}

bool DiffusionCurveRenderer::Shader::Bind()
{
    return mProgram->bind();
}

void DiffusionCurveRenderer::Shader::Release()
{
    mProgram->release();
}

void DiffusionCurveRenderer::Shader::AddPath(QOpenGLShader::ShaderTypeBit type, const QString& path)
{
    mPaths.insert(type, path);
}

void DiffusionCurveRenderer::Shader::AddUniform(const QString& uniform)
{
    mUniforms << uniform;
}

void DiffusionCurveRenderer::Shader::AddUniforms(const QStringList& uniforms)
{
    mUniforms << uniforms;
}

void DiffusionCurveRenderer::Shader::SetUniformArray(const QString& uniform, int size)
{
    mUniformArrays.insert(uniform, size);
}

void DiffusionCurveRenderer::Shader::AddAttribute(const QString& attribute)
{
    mAttributes << attribute;
}

void DiffusionCurveRenderer::Shader::AddAttributes(const QStringList& attributes)
{
    mAttributes << attributes;
}

void DiffusionCurveRenderer::Shader::SetUniformValue(const QString& name, int value)
{
    mProgram->setUniformValue(mLocations[name], value);
}

void DiffusionCurveRenderer::Shader::SetUniformValue(const QString& name, unsigned int value)
{
    mProgram->setUniformValue(mLocations[name], value);
}

void DiffusionCurveRenderer::Shader::SetUniformValue(const QString& name, float value)
{
    mProgram->setUniformValue(mLocations[name], value);
}

void DiffusionCurveRenderer::Shader::SetUniformValue(const QString& name, const QVector3D& value)
{
    mProgram->setUniformValue(mLocations[name], value);
}

void DiffusionCurveRenderer::Shader::SetUniformValue(const QString& name, const QVector4D& value)
{
    mProgram->setUniformValue(mLocations[name], value);
}

void DiffusionCurveRenderer::Shader::SetUniformValue(const QString& name, const QMatrix4x4& value)
{
    mProgram->setUniformValue(mLocations[name], value);
}

void DiffusionCurveRenderer::Shader::SetUniformValue(const QString& name, const QMatrix3x3& value)
{
    mProgram->setUniformValue(mLocations[name], value);
}

void DiffusionCurveRenderer::Shader::SetUniformValueArray(const QString& name, const QVector<float>& values)
{
    mProgram->setUniformValueArray(mLocations[name], values.constData(), values.size(), 1);
}

void DiffusionCurveRenderer::Shader::SetUniformValueArray(const QString& name, const QVector<QVector4D>& values)
{
    mProgram->setUniformValueArray(mLocations[name], values.constData(), values.size());
}

void DiffusionCurveRenderer::Shader::SetUniformValueArray(const QString& name, const QVector<QVector3D>& values)
{
    mProgram->setUniformValueArray(mLocations[name], values.constData(), values.size());
}

void DiffusionCurveRenderer::Shader::SetUniformValueArray(const QString& name, const QVector<QVector2D>& values)
{
    mProgram->setUniformValueArray(mLocations[name], values.constData(), values.size());
}

void DiffusionCurveRenderer::Shader::SetSampler(const QString& name, unsigned int unit, unsigned int id, GLenum target)
{
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(target, id);
    SetUniformValue(name, unit);
}

ShaderType DiffusionCurveRenderer::Shader::GetType() const
{
    return mType;
}

QString DiffusionCurveRenderer::Shader::GetShaderTypeString(QOpenGLShader::ShaderTypeBit type)
{
    switch (type)
    {
    case QOpenGLShader::Vertex:
        return "Vertex Shader";
    case QOpenGLShader::Fragment:
        return "Fragment Shader";
    case QOpenGLShader::Geometry:
        return "Geometry Shader";
    case QOpenGLShader::TessellationControl:
        return "Tessellation Control Shader";
    case QOpenGLShader::TessellationEvaluation:
        return "Tesselation Evaluation Shader";
    case QOpenGLShader::Compute:
        return "Compute Shader";
    default:
        return "Unknown Shader";
    }
}

QString DiffusionCurveRenderer::Shader::GetShaderTypeString()
{
    switch (mType)
    {
    default:
        return "Unknown Shader";
    }
}