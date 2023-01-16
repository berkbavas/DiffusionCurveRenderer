#pragma once

#include "Common.h"
#include "Manager.h"
#include <QOpenGLFunctions>

#include <QMatrix3x3>

namespace DiffusionCurveRenderer
{
    class Shader;

    class ShaderManager : public Manager
    {
    private:
        explicit ShaderManager(QObject* parent = nullptr);

    public:
        bool Init() override;
        bool Bind(ShaderType shader);
        void Release();

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

        static ShaderManager* Instance();

    private:
        ShaderType mActiveShader;
        QMap<ShaderType, Shader*> mShaders;
    };
}
