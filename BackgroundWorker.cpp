#include "BackgroundWorker.h"
#include "MainWindow.h"
#include "RenderParams.h"

#include <complex>
#include <thread>
#include <chrono>
#include <cassert>
#include <iostream>
#include <mutex>
#include <iostream>

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

BackgroundWorker::BackgroundWorker(QWidget* parent) :
    QObject(parent),
    m_monitorThread(nullptr),
    m_state(STOPPED),
    m_threadMutexes(std::thread::hardware_concurrency())
{
    connect(this, SIGNAL(workerDone()), this, SLOT(cleanup()), Qt::QueuedConnection);
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
        m_state = CANCELED;

        assert(m_monitorThread != nullptr);

        if (m_monitorThread->joinable()) {
            m_monitorThread->join();
        }
    }

    cleanup();
}

void BackgroundWorker::cleanup()
{
    if (m_monitorThread == nullptr) {
        return;
    }

    if (m_monitorThread->joinable()) {
        m_monitorThread->join();
    }

    delete m_monitorThread;
    m_monitorThread = nullptr;
    assert(m_workerThreads.empty());

    if (m_state == CANCELED) {
        m_state = STOPPED;
        emit taskComplete(true);
    } else {
        m_state = STOPPED;
        emit taskComplete(false);
    }
}

void BackgroundWorker::task(QImage* image, const RenderParams& params, int& currentLine, int threadIndex)
{
    int y = 0;
    uchar* pixPtr = nullptr;
    int width = image->width();
    int height = image->height();
    const ZoomRegion& region = params.zoomRegion();

    while (true) {
        {
            std::unique_lock<std::mutex> lock(this->m_lineMutex);

            if (currentLine >= height) {
                break;
            }

            y = currentLine++;
            pixPtr = image->scanLine(y);

            int progress = (int) ((double) y / (double) height * 100);
            emit progressUpdate(progress);
        }

        {
            std::unique_lock<std::mutex> lock(this->m_threadMutexes[threadIndex]);

            double imag = (double) y / (double) (height - 1) * region.height() + region.location().y();

            //TODO: implement antialiasing

            for (int x = 0; x < width; x++) {
                double real = (double) x / (double) (width - 1) * region.width() + region.location().x();

                std::complex<double> c(real, imag);

                int numIters = mandelbrot(c, 32, 2.0);

                int color = 0;

                if (numIters >= 0) {
                    //TODO: use color maps
                    color = static_cast<int>(std::min((double) numIters / 31.0 * 255.0 + 0.5, 255.0));
                }

                *(QRgb*)pixPtr = qRgb(color, color, color);
                pixPtr += sizeof(QRgb);
            }
        }

        if (this->m_state == CANCELED) {
            return;
        }
    }
}

void BackgroundWorker::run(QImage* image, const RenderParams& params)
{
    assert(m_state == STOPPED);
    m_state = RUNNING;

    emit taskStart();

    m_monitorThread = new std::thread([this, image, params]() {
        int currentLine = 0;

        auto boundTask = [this, image, &params, &currentLine](int threadIndex) {
            this->task(image, params, currentLine, threadIndex);
        };

        for (int i = 0; i < std::thread::hardware_concurrency(); i++) {
            m_workerThreads.emplace_back(boundTask, i);
        }

        for (std::thread& thread : m_workerThreads) {
            thread.join();
        }

        m_workerThreads.clear();

        emit workerDone();
    });
}

std::mutex& BackgroundWorker::threadMutex(int threadNum)
{
    return m_threadMutexes[threadNum];
}
