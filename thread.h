#ifndef THREAD_H
#define THREAD_H

#include <pthread.h>
#include <assert.h>

class ThreadManager;


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

protected:
    bool b_mustStop;

private:
    virtual void *run() = 0;

    pthread_t thread;
};

#endif // THREAD_H
