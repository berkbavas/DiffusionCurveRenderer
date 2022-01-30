#ifndef CONTROLPOINT_H
#define CONTROLPOINT_H

#include <QObject>
#include <QVector2D>

class ControlPoint : public QObject
{
public:
    ControlPoint(QVector2D);

    const QVector2D &position() const;
    void setPosition(const QVector2D &newPosition);

    bool selected() const;
    void setSelected(bool newSelected);

    int index() const;
    void setIndex(int newIndex);

private:
    QVector2D mPosition;
    bool mSelected;
    int mIndex;
};

#endif // CONTROLPOINT_H
