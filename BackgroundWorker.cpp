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
    double mandelbrot(const double c_real, const double c_imag, const int maxIters, const double boundary)
    {
        const double boundarySqr = boundary * boundary;

        double z_real = c_real;
        double z_imag = c_imag;

        for (int i = 0; i < maxIters; i++) {
            double z_real_sqr = z_real * z_real;
            double z_imag_sqr = z_imag * z_imag;
            double z_real_imag = z_real * z_imag;

            double z_mag_sqr = z_real_sqr + z_imag_sqr;

            if (z_mag_sqr > boundarySqr) {
                return i;
            }

            z_real = z_real_sqr - z_imag_sqr + c_real;
            z_imag = z_real_imag + z_real_imag + c_imag;
        }

        return -1;
    }

    const int ITERS = 64;
    const int ANTIALIASING = 4;
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
    int antialiasing = params.antialiasing();

    double recip_antialiasing_plus_1 = 1.0 / (double) (antialiasing + 1);
    double region_width_over_width_minus_1 = (double) region.width() / (double) (width - 1);
    double region_height_over_height_minus_1 = (double) region.height() / (double) (height - 1);
    double recip_iters_minus_1 = 1.0 / (double) (ITERS - 1);
    double recip_antialiasing_square = 1.0 / (double) (antialiasing * antialiasing);

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

            for (int x = 0; x < width; x++) {
                double totalColor = 0;

                for (int aay = 0; aay < antialiasing; aay++) {
                    double y_offset = (double) aay * recip_antialiasing_plus_1 - 0.5;
                    double imag = ((double) y + y_offset) * region_height_over_height_minus_1 + region.location().y();

                    for (int aax = 0; aax < antialiasing; aax++) {
                        double x_offset = (double) aax * recip_antialiasing_plus_1 - 0.5;
                        double real = ((double) x + x_offset) * region_width_over_width_minus_1 + region.location().x();

                        int numIters = mandelbrot(real, imag, ITERS, 2.0);

                        if (numIters >= 0) {
                            //TODO: use color maps
                            totalColor += (double) numIters * recip_iters_minus_1;
                        }
                    }
                }

                int color = (int) ((double) totalColor * recip_antialiasing_square * 255.0 + 0.5);

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

        for (unsigned int i = 0; i < std::thread::hardware_concurrency(); i++) {
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
