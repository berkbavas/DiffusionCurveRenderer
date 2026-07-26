#pragma once

#include "Util/Logger.h"

#include <map>

#include <QObject>
#include <QOpenGLExtraFunctions>
#include <QOpenGLShader>

namespace DiffusionCurveRenderer
{
    class Shader : protected QOpenGLExtraFunctions
    {
      public:
        Shader(const QString& Name);

        void Initialize();
        bool Bind();
        void Release();

        void AddPath(QOpenGLShader::ShaderTypeBit Type, const QString& Path);

        QString GetName() const;

        static QString GetShaderTypeString(QOpenGLShader::ShaderTypeBit Type);

        template<typename T>
        void SetUniformValue(const QString& Name, T Value)
        {
            const auto Location = mProgram->uniformLocation(Name);

            if (0 <= Location)
            {
                mProgram->setUniformValue(Location, Value);
            }
            else
            {
                LOG_WARN("Shader::SetUniformValue: Uniform location '{}' could not be found.", Name.toStdString());
            }
        }

        template<typename T>
        void SetUniformValueArray(const QString& Name, const QVector<T>& Values)
        {
            const auto Location = mProgram->uniformLocation(Name);

            if (0 <= Location)
            {
                mProgram->setUniformValueArray(Location, Values.constData(), Values.size());
            }
            else
            {
                LOG_WARN("Shader::SetUniformValue: Uniform location '{}' could not be found.", Name.toStdString());
            }
        }

        void SetUniformValueFloatArray(const QString& Name, const QVector<float>& Values)
        {
            const auto Location = mProgram->uniformLocation(Name);

            if (0 <= Location)
            {
                mProgram->setUniformValueArray(Location, Values.constData(), Values.size(), 1);
            }
            else
            {
                LOG_WARN("Shader::SetUniformValue: Uniform location '{}' could not be found.", Name.toStdString());
            }
        }

        void SetSampler(const QString& Name, GLuint Unit, GLuint TextureId, GLuint Target = GL_TEXTURE_2D);

      private:
        QSharedPointer<QOpenGLShaderProgram> mProgram;
        std::map<QOpenGLShader::ShaderTypeBit, QString> mPaths;

        QString mName;
    };
}
