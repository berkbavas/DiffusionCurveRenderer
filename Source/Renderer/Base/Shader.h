#pragma once

#include "Util/Logger.h"

#include <QObject>
#include <QOpenGLExtraFunctions>
#include <QOpenGLShader>
#include <map>

namespace DiffusionCurveRenderer
{
    class Shader : protected QOpenGLExtraFunctions
    {
      public:
        Shader(const QString& name);

        void Initialize();
        bool Bind();
        void Release();

        void AddPath(QOpenGLShader::ShaderTypeBit type, const QString& path);

        QString GetName() const;

        static QString GetShaderTypeString(QOpenGLShader::ShaderTypeBit type);

        template<typename T>
        void SetUniformValue(const QString& name, T value)
        {
            const auto location = mProgram->uniformLocation(name);

            if (0 <= location)
            {
                mProgram->setUniformValue(location, value);
            }
            else
            {
                LOG_WARN("Shader::SetUniformValue: Uniform location '{}' could not be found.", name.toStdString());
            }
        }

        template<typename T>
        void SetUniformValueArray(const QString& name, const QVector<T>& values)
        {
            const auto location = mProgram->uniformLocation(name);

            if (0 <= location)
            {
                mProgram->setUniformValueArray(location, values.constData(), values.size());
            }
            else
            {
                LOG_WARN("Shader::SetUniformValue: Uniform location '{}' could not be found.", name.toStdString());
            }
        }

        void SetUniformValueFloatArray(const QString& name, const QVector<float>& values)
        {
            const auto location = mProgram->uniformLocation(name);

            if (0 <= location)
            {
                mProgram->setUniformValueArray(location, values.constData(), values.size(), 1);
            }
            else
            {
                LOG_WARN("Shader::SetUniformValue: Uniform location '{}' could not be found.", name.toStdString());
            }
        }

        void SetSampler(const QString& name, GLuint unit, GLuint textureId, GLuint target = GL_TEXTURE_2D);

      private:
        QSharedPointer<QOpenGLShaderProgram> mProgram;
        std::map<QOpenGLShader::ShaderTypeBit, QString> mPaths;

        QString mName;
    };
}
