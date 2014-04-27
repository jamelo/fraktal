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

#include <cassert>
#include <iostream>

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
    m_colors(ColorScheme::Fire),
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
    connect(m_refreshTimer, SIGNAL(timeout()), this, SLOT(refreshPreview()));
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
    //TODO: double click zooming
    QWidget::mouseDoubleClickEvent ( event );
}

void Canvas::wheelEvent ( QWheelEvent* event ) {
    if (event->orientation() == Qt::Orientation::Vertical) {
        //TODO: scroll wheel zooming
    } else {
        QWidget::wheelEvent ( event );
    }
}

void Canvas::mousePressEvent ( QMouseEvent* event ) {
    if (event->button() == Qt::LeftButton) {
        m_panning = true;
        m_zooming = false;
        m_dragStart = event->pos();
    } else if (event->button() == Qt::RightButton) {
        m_zooming = true;
        m_panning = false;
        m_dragStart = event->pos();
    }
}

void Canvas::mouseReleaseEvent ( QMouseEvent* event ) {
    if (event->button() == Qt::LeftButton) {
        m_panning = false;
        render();
    } else if (event->button() == Qt::RightButton) {
        m_zooming = false;
        render();
    }
}

void Canvas::mouseMoveEvent ( QMouseEvent* event ) {
    if (m_panning) {
        int mouseDelta_x = event->pos().x() - m_dragStart.x();
        int mouseDelta_y = event->pos().y() - m_dragStart.y();

        m_dragStart = event->pos();

        double delta_x = -(double) mouseDelta_x / (double) this->width() * m_region.width();
        double delta_y = -(double) mouseDelta_y / (double) this->height() * m_region.height();

        Point newCenter = Point(m_region.center().x() + delta_x, m_region.center().y() + delta_y);
        m_region = ZoomRegion(newCenter, m_region.width(), m_region.height());

        renderSketch();
    }

    if (m_zooming) {
        int mouseDelta_y = event->pos().y() - m_dragStart.y();

        m_dragStart = event->pos();

        double zoom = std::exp((double) mouseDelta_y / (double) this->height() * 5.0);

        double width = m_region.width() * zoom;
        double height = m_region.height() * zoom;

        //TODO: do math to recalculate center
        m_region = ZoomRegion(m_region.center(), width, height);

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

    {
        std::vector<std::unique_lock<std::mutex>> locks;

        for (int i = 0; i < m_worker->threadCount(); i++) {
            locks.emplace_back(m_worker->threadMutex(i));
        }

        pixmap = QPixmap::fromImage(m_image);
    }

    if (pixmap.width() == this->width() && pixmap.height() == this->height()) {
        this->setPixmap(pixmap);
    } else {
        QPixmap pixmapScaled = pixmap.scaled(this->width(), this->height(), Qt::IgnoreAspectRatio, Qt::FastTransformation);
        this->setPixmap(pixmapScaled);
    }
}


