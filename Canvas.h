#ifndef Canvas_H
#define Canvas_H

#include <QLabel>
#include <QImage>

#include "ZoomRegion.h"
#include "ColorScheme.h"

class BackgroundWorker;

class Canvas : public QLabel
{
    Q_OBJECT

    private:
        QTimer* m_resizeTimer;
        QTimer* m_refreshTimer;
        ZoomRegion m_region;
        ColorScheme m_colors;
        int m_antialiasing;
        BackgroundWorker* m_worker;
        QImage m_image;

        bool m_panning;
        bool m_zooming;
        QPoint m_dragStart;

        void render();
        void renderSketch();

    public:
        Canvas(QWidget* parent);
        virtual ~Canvas();

        BackgroundWorker* backgroundWorker() { return m_worker; }

    protected:
        virtual void resizeEvent(QResizeEvent* event);
        virtual void wheelEvent(QWheelEvent* event);
        virtual void mouseDoubleClickEvent(QMouseEvent* event);
        virtual void resizeComplete();
        virtual void mousePressEvent(QMouseEvent* event);
        virtual void mouseReleaseEvent(QMouseEvent* event);
        virtual void mouseMoveEvent(QMouseEvent* event);

    private slots:
        void resizeTimerExpired();
        void renderComplete(bool canceled);
        void refreshPreview();
};

#endif
