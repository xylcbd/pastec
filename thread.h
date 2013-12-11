#ifndef THREAD_H
#define THREAD_H

#include <pthread.h>
#include <assert.h>


class Thread
{
    friend class ThreadManager;

public:
    Thread() : b_mustStop(false) {}
    void join()
    {
        void *res;
        assert(pthread_join(thread, &res) == 0);
    }

    bool start()
    {
        pthread_create(&thread, NULL, &runThread, this);
        return true;
    }

protected:
    static void *runThread(void *p)
    {
        Thread *newThread = (Thread *)p;
        newThread->b_mustStop = false;
        return newThread->run();
    }

    bool b_mustStop;

private:
    virtual void *run() = 0;

    pthread_t thread;
};

#endif // THREAD_H
