#ifndef BackgroundWorker_H
#define BackgroundWorker_H

#include <thread>
#include <vector>
#include <mutex>

#include <QObject>

class MainWindow;

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
        State m_state;
        std::mutex m_lineMutex;

    signals:
        void taskComplete();
        void progressUpdate(int);

    private slots:
        void cleanup();

    public:
        BackgroundWorker(MainWindow* parent);
        virtual ~BackgroundWorker();

        void run();
        void operator()();
        void cancel();
};

#endif
