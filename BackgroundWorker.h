#ifndef BackgroundWorker_H
#define BackgroundWorker_H

#include <thread>
#include <vector>
#include <mutex>

#include <QObject>

class MainWindow;
class RenderParams;
class QImage;

class BackgroundWorker : public QObject
{
    Q_OBJECT

    public:
        enum State
        {
            STOPPED,
            RUNNING,
            CANCELED
        };

    private:
        std::thread* m_monitorThread;
        std::vector<std::thread> m_workerThreads;
        std::vector<std::mutex> m_threadMutexes;
        State m_state;
        std::mutex m_lineMutex;
        std::mutex m_stateMutex;
        std::mutex m_startLock;

        void task(QImage* image, const RenderParams& params, int& currentLine, int threadIndex);

    signals:
        void taskStart();
        void taskComplete(bool);
        void progressUpdate(int);
        void workerDone();

    private slots:
        void cleanup();

    public:
        BackgroundWorker(QWidget* parent);
        virtual ~BackgroundWorker();

        void run(QImage* image, const RenderParams& params);
        void cancel();
        std::mutex& threadMutex(int threadNum);
        int threadCount() const { return m_workerThreads.size(); }
};

#endif
