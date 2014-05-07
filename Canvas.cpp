#include "Canvas.h"

#include <QResizeEvent>
#include <QMouseEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QWheelEvent>
#include <QLabel>
#include <QTimer>
#include <QImage>
#include <QPixmap>
#include <QApplication>

#include <cassert>
#include <iostream>
#include <cmath>

#include "ZoomRegion.h"
#include "BackgroundWorker.h"
#include "RenderParams.h"
#include "ColorScheme.h"

namespace {
    const int RESIZE_DELAY = 250;
    const int REFRESH_DELAY = 100;
}

Canvas::Canvas ( QWidget* parent ) :
    QLabel(parent),
    m_resizeTimer(nullptr),
    m_refreshTimer(nullptr),
    m_region(-2.0, -1.0, 1.0, 1.0),
    m_colors(ColorScheme::Rainbow),
    m_antialiasing(1),
    m_worker(nullptr),
    m_image()
{
    this->setScaledContents(true);
    this->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    QPixmap pixmap(1, 1);
    pixmap.fill(Qt::black);
    this->setPixmap(pixmap);

    m_worker = new BackgroundWorker(this);

    m_resizeTimer = new QTimer(this);
    m_resizeTimer->setInterval(RESIZE_DELAY);
    m_resizeTimer->setSingleShot(true);

    m_refreshTimer = new QTimer(this);
    m_refreshTimer->setInterval(REFRESH_DELAY);

    connect(m_resizeTimer, SIGNAL(timeout()), this, SLOT(resizeTimerExpired()));
    //connect(m_refreshTimer, SIGNAL(timeout()), this, SLOT(refreshPreview()));
    connect(m_worker, SIGNAL(taskComplete(bool)), this, SLOT(renderComplete(bool)));

    render();
}

Canvas::~Canvas() {
    m_worker->cancel();
}

void Canvas::resizeEvent ( QResizeEvent* event ) {
    m_resizeTimer->stop();
    m_resizeTimer->start();
    QWidget::resizeEvent ( event );
}

void Canvas::resizeComplete() {
    render();
}

void Canvas::mouseDoubleClickEvent ( QMouseEvent* event ) {
    const double zoom = 1.0 / 4.0;

    double width = (double) m_region.width() * zoom;
    double height = (double) m_region.height() * zoom;

    double zoomPoint_x = (double) event->pos().x() / (double) this->width() * m_region.width() + m_region.location().x();
    double zoomPoint_y = (double) event->pos().y() / (double) this->height() * m_region.height() + m_region.location().y();

    m_region = ZoomRegion(Point(zoomPoint_x, zoomPoint_y), width, height);
}

void Canvas::wheelEvent ( QWheelEvent* event ) {
    if (event->orientation() == Qt::Orientation::Vertical) {
        double zoom = event->delta() > 0 ? 1.0 / 1.25 : 1.25;

        double width = (double) m_region.width() * zoom;
        double height = (double) m_region.height() * zoom;

        double zoomPoint_x = (double) event->pos().x() / (double) this->width() * m_region.width() + m_region.location().x();
        double zoomPoint_y = (double) event->pos().y() / (double) this->height() * m_region.height() + m_region.location().y();

        double center_x = zoomPoint_x - (zoomPoint_x - m_region.center().x()) * zoom;
        double center_y = zoomPoint_y - (zoomPoint_y - m_region.center().y()) * zoom;
        m_region = ZoomRegion(Point(center_x, center_y), width, height);

        renderSketch();
        m_resizeTimer->start();
    } else {
        QWidget::wheelEvent ( event );
    }
}

void Canvas::mousePressEvent ( QMouseEvent* event ) {
    if (event->button() == Qt::LeftButton) {
        m_panning = true;
        m_zooming = false;
        m_dragLast = event->pos();
    } else if (event->button() == Qt::RightButton) {
        m_zooming = true;
        m_panning = false;
        m_dragLast = event->pos();

        double zoomPivot_x = (double) event->pos().x() / (double) this->width() * m_region.width() + m_region.location().x();
        double zoomPivot_y = (double) event->pos().y() / (double) this->height() * m_region.height() + m_region.location().y();
        m_zoomPivot = Point(zoomPivot_x, zoomPivot_y);

        QApplication::setOverrideCursor(Qt::BlankCursor);
    }
}

void Canvas::mouseReleaseEvent ( QMouseEvent* event ) {
    if (event->button() == Qt::LeftButton) {
        m_panning = false;
        render();
    } else if (event->button() == Qt::RightButton) {
        m_zooming = false;
        render();

        QApplication::restoreOverrideCursor();
    }
}

void Canvas::mouseMoveEvent ( QMouseEvent* event ) {
    if (m_panning) {
        int mouseDelta_x = event->pos().x() - m_dragLast.x();
        int mouseDelta_y = event->pos().y() - m_dragLast.y();

        m_dragLast = event->pos();

        double delta_x = -(double) mouseDelta_x / (double) this->width() * m_region.width();
        double delta_y = -(double) mouseDelta_y / (double) this->height() * m_region.height();

        Point newCenter = Point(m_region.center().x() + delta_x, m_region.center().y() + delta_y);
        m_region = ZoomRegion(newCenter, m_region.width(), m_region.height());

        renderSketch();
    }

    if (m_zooming) {
        int mouseDelta_y = event->pos().y() - m_dragLast.y();

        QCursor::setPos(this->mapToGlobal(m_dragLast));

        double zoom = std::exp((double) mouseDelta_y / (double) this->height() * 5.0);

        double width = (double) m_region.width() * zoom;
        double height = (double) m_region.height() * zoom;

        double center_x = m_zoomPivot.x() - (m_zoomPivot.x() - m_region.center().x()) * zoom;
        double center_y = m_zoomPivot.y() - (m_zoomPivot.y() - m_region.center().y()) * zoom;
        m_region = ZoomRegion(Point(center_x, center_y), width, height);

        renderSketch();
    }
}

void Canvas::resizeTimerExpired() {
    m_resizeTimer->stop();
    resizeComplete();
}

void Canvas::render()
{
    RenderParams params(m_region, m_colors, m_antialiasing);

    m_worker->cancel();

    m_image = this->pixmap()->scaled(this->width(), this->height(), Qt::IgnoreAspectRatio, Qt::FastTransformation).toImage();
    m_worker->run(&m_image, params);

    m_refreshTimer->start();

    emit rendering();
}

void Canvas::renderSketch()
{
    RenderParams params(m_region, m_colors, 1);

    m_worker->cancel();

    m_image = QImage(this->width() / 4, this->height() / 4, QImage::Format_ARGB32);
    m_worker->run(&m_image, params);
}


void Canvas::renderComplete(bool canceled)
{
    m_refreshTimer->stop();

    if (!canceled) {
        refreshPreview();
    }
}

void Canvas::refreshPreview()
{
    QPixmap pixmap;

    //TODO: blit completed lines of image to screen only rather than locking all render threads to copy entire image

    {
        std::vector<std::unique_lock<std::mutex>> locks;

        for (int i = 0; i < m_worker->threadCount(); i++) {
            locks.emplace_back(m_worker->threadMutex(i));
        }

        pixmap = QPixmap::fromImage(m_image);

        /*QImage partialImage(m_image.data_ptr(), m_image.width(), m_linesCompleted, m_image.bytesPerLine(), m_image.format());
        pixmap = QPixmap::fromImage(partialImage);
        QPixmap pixmap2(m_image.width(), m_image.height());
        pixmap2.fill(Qt::black);
        pixmap2.loadFromData(m_image.data_ptr(), m_image.format(), Qt::)*/
    }

    if (pixmap.width() == this->width() && pixmap.height() == this->height()) {
        this->setPixmap(pixmap);
    } else {
        QPixmap pixmapScaled = pixmap.scaled(this->width(), this->height(), Qt::IgnoreAspectRatio, Qt::FastTransformation);
        this->setPixmap(pixmapScaled);
    }
}

void Canvas::setAntialiasing ( int antialiasing ) {
    m_antialiasing = antialiasing;
    render();
}

int Canvas::antialiasing() {
    return m_antialiasing;
}

void Canvas::setColorScheme ( const ColorScheme& colors ) {
    m_colors = colors;
    render();
}

const ColorScheme& Canvas::colorScheme() {
    return m_colors;
}
