#pragma once

#include "Bezier.h"
#include "EditModeCamera.h"
#include "Manager.h"

#include <QFuture>
#include <QTimer>

namespace DiffusionCurveRenderer
{
    class CurveManager : public Manager
    {
        explicit CurveManager(QObject* parent = nullptr);

    public:
        static CurveManager* Instance();
        bool Init() override;

        // Actions
        void Select(RenderMode renderMode, const QVector2D& position, float radius = 5.0f);
        void AddControlPoint(const QVector2D& position, bool select = true);
        void AddColorPoint(const QVector2D& position, bool select = true);
        void AddBlurPoint(const QVector2D& position, bool select = true);

        void RemoveSelectedCurve();
        void RemoveSelectedControlPoint();
        void RemoveSelectedColorPoint();
        void RemoveSelectedBlurPoint();

        void AddCurve(Bezier* curve);
        void AddCurves(const QList<Bezier*> curves);

        void RemoveCurve(int index);
        void RemoveCurve(Curve* curve);

        void SetGlobalContourThickness(float thickness);
        void SetGlobalContourColor(const QVector4D& color);
        void SetGlobalDiffusionWidth(float width);
        void SetGlobalDiffusionGap(float gap);
        void SetGlobalBlurStrength(float strength);

        void DeselectAllCurves();

        ControlPoint* GetClosestControlPointOnSelectedCurve(const QVector2D& nearbyPoint, float radius = 20.0f) const;
        ColorPoint* GetClosestColorPointOnSelectedCurve(const QVector2D& nearbyPoint, float radius = 5.0f) const;
        BlurPoint* GetClosestBlurPointOnSelectedCurve(const QVector2D& nearbyPoint, float radius = 5.0f) const;

        const QList<Bezier*>& GetCurves() const;

        Bezier* GetSelectedCurve() const;
        void SetSelectedCurve(Bezier* newSelectedCurve);

        ControlPoint* GetSelectedControlPoint() const;
        void SetSelectedControlPoint(ControlPoint* newSelectedControlPoint);

        ColorPoint* GetSelectedColorPoint() const;
        void SetSelectedColorPoint(ColorPoint* newSelectedColorPoint);

        BlurPoint* GetSelectedBlurPoint() const;
        void SetSelectedBlurPoint(BlurPoint* newSelectedBlurPoint);

        void SortCurves();
        void Clear();

        void MakeVoid(float threshold);

    private:
        void SelectCurve(QVector2D position, float radius = 20.0f);

    private:
        EditModeCamera* mCamera;
        QList<Bezier*> mCurves;
        Bezier* mSelectedCurve;
        ControlPoint* mSelectedControlPoint;
        ColorPoint* mSelectedColorPoint;
        BlurPoint* mSelectedBlurPoint;
    };
}
