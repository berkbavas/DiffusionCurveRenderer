#pragma once

#include <QObject>

namespace DiffusionCurveRenderer
{
    class Manager : public QObject
    {
        Q_OBJECT
    public:
        explicit Manager(QObject* parent = nullptr);

        virtual bool Init() = 0;
    };
}
