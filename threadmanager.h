#ifndef THREADMANAGER_H
#define THREADMANAGER_H

#include <pthread.h>
#include <list>

#include "server.h"

using namespace std;

class Thread;


class ThreadManager
{
public:
    ThreadManager();

    bool startThread(Thread *t);
    void joinThreads();

private:
    static void *runThread(void *p);
};

#endif // THREADMANAGER_H
