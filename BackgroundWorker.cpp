#include "BackgroundWorker.h"
#include "MainWindow.h"

#include <complex>
#include <thread>
#include <chrono>
#include <cassert>
#include <iostream>
#include <mutex>

namespace
{
    int mandelbrot(const std::complex<double>& c, int maxIters, double boundary)
    {
        std::complex<double> z(0.0, 0.0);

        for (int i = 0; i < maxIters; i++) {
            z = z * z + c;

            if (std::abs(z) > boundary) {
                return i;
            }
        }

        return -1;
    }
}

BackgroundWorker::BackgroundWorker(MainWindow* parent) :
    QObject(parent),
    m_monitorThread(nullptr),
    m_state(STOPPED)
{
    connect(this, SIGNAL(taskComplete()), this, SLOT(cleanup()), Qt::QueuedConnection);
}

BackgroundWorker::~BackgroundWorker()
{
    if (m_monitorThread != nullptr) {
        if (m_monitorThread->joinable()) {
            m_monitorThread->join();
        }

        delete m_monitorThread;
    }
}

void BackgroundWorker::cancel()
{
    if (m_state != STOPPED) {
        assert(m_monitorThread != nullptr);
        m_state = CANCELED;

        if (m_monitorThread->joinable()) {
            m_monitorThread->join();
        }
    }
}

void BackgroundWorker::cleanup()
{
    assert(m_monitorThread != nullptr);

    if (m_monitorThread->joinable()) {
        m_monitorThread->join();
    }

    delete m_monitorThread;
    m_monitorThread = nullptr;
    assert(m_workerThreads.empty());
    m_state = STOPPED;
}

void BackgroundWorker::run()
{
    assert(m_state == STOPPED);
    m_state = RUNNING;

    m_monitorThread = new std::thread([this]() {
        int currentLine = 0;

        for (int i = 0; i < std::thread::hardware_concurrency(); i++) {
            m_workerThreads.emplace_back([this, &currentLine]() {
                MainWindow* mwnd = qobject_cast<MainWindow*>(this->parent());
                int width = mwnd->m_image->width();
                int height = mwnd->m_image->height();
                ZoomRegion zoomRegion(-2, -1, 1, 1);
                int y = 0;

                while (true) {
                    {
                        std::unique_lock<std::mutex>(this->m_lineMutex);

                        if (currentLine >= height) {
                            break;
                        }

                        y = currentLine;
                        currentLine++;

                        int progress = (int) ((double) y / (double) height * 100);
                        emit progressUpdate(progress);
                    }

                    double imag = (double) y / (double) (height - 1) * zoomRegion.height() + zoomRegion.location().y();

                    for (int x = 0; x < width; x++) {
                        double real = (double) x / (double) (width - 1) * zoomRegion.width() + zoomRegion.location().x();

                        std::complex<double> c(real, imag);

                        int numIters = mandelbrot(c, 32, 2.0);

                        int color = 0;

                        if (numIters >= 0) {
                            color = static_cast<int>(std::min((double) numIters / 31.0 * 255.0 + 0.5, 255.0));
                        }

                        mwnd->m_image->setPixel(x, y, qRgb(color, color, color));
                    }

                    std::this_thread::sleep_for(std::chrono::milliseconds(30));

                    if (this->m_state == CANCELED) {
                        break;
                    }
                }
            });
        }

        for (std::thread& thread : m_workerThreads) {
            thread.join();
        }

        m_workerThreads.clear();

        emit taskComplete();
    });
}
