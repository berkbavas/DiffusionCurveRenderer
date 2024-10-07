#include "Shader.h"

#include "Util/Util.h"

#include <QDebug>
#include <QFile>

DiffusionCurveRenderer::Shader::Shader(const QString& name)
    : mProgram(nullptr)
    , mName(name)
{
}

void DiffusionCurveRenderer::Shader::Initialize()
{
    LOG_INFO("Shader::Initialize: '{}' is being initializing.", mName.toStdString());

    initializeOpenGLFunctions();

    mProgram = QSharedPointer<QOpenGLShaderProgram>(new QOpenGLShaderProgram);

    for (const auto [shaderType, path] : mPaths)
    {
        const auto bytes = Util::GetBytes(path);
        if (!mProgram->addShaderFromSourceCode(shaderType, bytes))
        {
            DCR_EXIT_FAILURE("Shader::Initialize: '{}' could not be loaded.", GetShaderTypeString(shaderType).toStdString());
        }
    }

    if (!mProgram->link())
    {
        DCR_EXIT_FAILURE("Shader::Initialize: Could not link shader program.");
    }

    if (!mProgram->bind())
    {
        DCR_EXIT_FAILURE("Shader::Initialize: Could not bind shader program.");
    }

    LOG_INFO("Shader::Initialize: '{}' has been initialized.", mName.toStdString());
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
    mPaths.emplace(type, path);
}

QString DiffusionCurveRenderer::Shader::GetName() const
{
    return mName;
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

void DiffusionCurveRenderer::Shader::SetSampler(const QString& name, GLuint unit, GLuint textureId, GLuint target)
{
    SetUniformValue(name, unit);
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(target, textureId);
}
