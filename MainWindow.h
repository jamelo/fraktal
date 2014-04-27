#ifndef MainWindow_H
#define MainWindow_H

#include <thread>

#include <KXmlGuiWindow>

#include "ColorScheme.h"
#include "Wrapper.h"
#include "BackgroundWorker.h"
#include "Point.h"
#include "ZoomRegion.h"

class QWidget;
class QLabel;
class Canvas;
class QProgressBar;
class QImage;

class MainWindow : public KXmlGuiWindow
{
    Q_OBJECT

    private:
        int m_antialiasingAmount;
        ColorScheme m_colorScheme;
        ZoomRegion m_zoomRegion;

        Canvas* m_canvas;
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
        void stop();
        void changeAntiAliasing(int amount);
        void changeColorScheme(QObject* colors);
        void customColorScheme();
        void previewStart();
        void previewComplete(bool canceled);
};

#endif
