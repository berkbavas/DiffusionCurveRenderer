#pragma once

#include "Common.h"
#include <QObject>
#include <QOpenGLFunctions>
#include <QOpenGLShader>

namespace DiffusionCurveRenderer
{
    class Shader : public QObject, protected QOpenGLFunctions
    {
        Q_OBJECT
    public:
        explicit Shader(ShaderType type, QObject* parent = nullptr);

        bool Init();
        bool Bind();
        void Release();

        void AddPath(QOpenGLShader::ShaderTypeBit type, const QString& path);
        void AddUniform(const QString& uniform);
        void AddUniforms(const QStringList& uniforms);
        void SetUniformArray(const QString& uniform, int size);
        void AddAttribute(const QString& attribute);
        void AddAttributes(const QStringList& attributes);

        void SetUniformValue(const QString& name, int value);
        void SetUniformValue(const QString& name, unsigned int value);
        void SetUniformValue(const QString& name, float value);
        void SetUniformValue(const QString& name, const QVector3D& value);
        void SetUniformValue(const QString& name, const QVector4D& value);
        void SetUniformValue(const QString& name, const QMatrix4x4& value);
        void SetUniformValue(const QString& name, const QMatrix3x3& value);
        void SetUniformValueArray(const QString& name, const QVector<float>& values);
        void SetUniformValueArray(const QString& name, const QVector<QVector4D>& values);
        void SetUniformValueArray(const QString& name, const QVector<QVector3D>& values);
        void SetUniformValueArray(const QString& name, const QVector<QVector2D>& values);
        void SetSampler(const QString& name, unsigned int unit, unsigned int id, GLenum target = GL_TEXTURE_2D);

        ShaderType GetType() const;

        QString GetShaderTypeString(QOpenGLShader::ShaderTypeBit type);
        QString GetShaderTypeString();

    private:
        QOpenGLShaderProgram* mProgram;
        QMap<QOpenGLShader::ShaderTypeBit, QString> mPaths;
        QMap<QString, GLuint> mLocations;

        ShaderType mType;
        QString mShaderName;

        QStringList mAttributes;
        QStringList mUniforms;
        QMap<QString, int> mUniformArrays;
    };
}
