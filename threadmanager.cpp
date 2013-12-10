#include "threadmanager.h"


ThreadManager::ThreadManager()
{ }


bool ThreadManager::startThread(Thread *t)
{
    pthread_create(&t->thread, NULL, &runThread, t);
    return true;
}


void *ThreadManager::runThread(void *p)
{
    Thread *newThread = (Thread *)p;
    newThread->b_mustStop = false;
    return newThread->run();
}
