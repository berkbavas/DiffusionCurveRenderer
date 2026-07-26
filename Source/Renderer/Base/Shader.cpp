#include "Shader.h"

#include "Util/Util.h"

#include <QDebug>
#include <QFile>

DiffusionCurveRenderer::Shader::Shader(const QString& Name)
    : mProgram(nullptr)
    , mName(Name)
{}

void DiffusionCurveRenderer::Shader::Initialize()
{
    LOG_INFO("Shader::Initialize: '{}' is being initializing.", mName.toStdString());

    initializeOpenGLFunctions();

    mProgram = QSharedPointer<QOpenGLShaderProgram>(new QOpenGLShaderProgram);

    for (const auto [ShaderType, Path] : mPaths)
    {
        const auto Bytes = Util::GetBytes(Path);
        if (!mProgram->addShaderFromSourceCode(ShaderType, Bytes))
        {
            DCR_EXIT_FAILURE("Shader::Initialize: '{}' could not be loaded.", GetShaderTypeString(ShaderType).toStdString());
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

void DiffusionCurveRenderer::Shader::AddPath(QOpenGLShader::ShaderTypeBit ShaderType, const QString& Path)
{
    mPaths.emplace(ShaderType, Path);
}

QString DiffusionCurveRenderer::Shader::GetName() const
{
    return mName;
}

QString DiffusionCurveRenderer::Shader::GetShaderTypeString(QOpenGLShader::ShaderTypeBit ShaderType)
{
    switch (ShaderType)
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
        return "Tessellation Evaluation Shader";
    case QOpenGLShader::Compute:
        return "Compute Shader";
    default:
        return "Unknown Shader";
    }
}

void DiffusionCurveRenderer::Shader::SetSampler(const QString& Name, GLuint Unit, GLuint TextureId, GLuint Target)
{
    SetUniformValue(Name, Unit);
    glActiveTexture(GL_TEXTURE0 + Unit);
    glBindTexture(Target, TextureId);
}
