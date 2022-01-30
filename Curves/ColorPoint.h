#ifndef COLORPOINT_H
#define COLORPOINT_H

#include <QColor>
#include <QObject>
#include <QVector4D>

class Curve;
class ColorPoint : public QObject
{
public:
    enum Type { Left = 0, Right = 1 };

    ColorPoint();

    QVector2D getPosition2D(float gap = 5) const;

    const QVector4D &color() const;
    void setColor(const QVector4D &newColor);

    float position() const;
    void setPosition(float newPosition);

    bool selected() const;
    void setSelected(bool newSelected);

    Type type() const;
    void setType(Type newType);

    Curve *parent() const;
    void setParent(Curve *newParent);

private:
    Curve *mParent;
    QVector4D mColor;
    float mPosition;
    bool mSelected;
    Type mType;
};

#endif // COLORPOINT_H
