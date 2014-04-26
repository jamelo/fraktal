#ifndef MainWindow_H
#define MainWindow_H

#include <thread>

#include <KXmlGuiWindow>

#include "ColorScheme.h"
#include "Wrapper.h"
#include "BackgroundWorker.h"

class QWidget;
class QLabel;
class QProgressBar;
class QImage;

class Point
{
    private:
        double m_x;
        double m_y;

    public:
        Point(double x, double y) :
            m_x(x),
            m_y(y)
        { }

        double x() { return m_x; }
        double y() { return m_y; }
};

class ZoomRegion
{
    private:
        double m_x1;
        double m_y1;
        double m_x2;
        double m_y2;

    public:
        ZoomRegion(double x1, double y1, double x2, double y2) :
            m_x1(x1),
            m_y1(y1),
            m_x2(x2),
            m_y2(y2)
        { }

        Point center()      { return Point((m_x1 + m_x2) / 2, (m_y1 + m_y2) / 2); }
        Point location()    { return Point(m_x1, m_y1); }
        double width()      { return m_x2 - m_x1; }
        double height()     { return m_y2 - m_y1; }
};

class MainWindow : public KXmlGuiWindow
{
    Q_OBJECT

    private:
        int m_antialiasingAmount;
        ColorScheme m_colorScheme;
        ZoomRegion m_zoomRegion;

        QLabel* m_canvas;
        QProgressBar* m_progressBar;
        QTimer* m_refreshTimer;
        QImage* m_image;
        BackgroundWorker* m_backgroundWorker;

        void setupActions();
        void setupWidgets();
        void setColorScheme();
        void renderPreview();

        friend class BackgroundWorker;

    public:
        MainWindow(QWidget* parent = nullptr);
        virtual ~MainWindow();

    private slots:
        void render();
        void saveAs();
        void zoomIn();
        void zoomOut();
        void zoomReset();
        void zoomArea();
        void stop();
        void changeAntiAliasing(int amount);
        void changeColorScheme(QObject* colors);
        void customColorScheme();
        void refreshPreview();
        void previewComplete();
};

#endif
