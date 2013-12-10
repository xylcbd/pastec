#ifndef SERVER_H
#define SERVER_H

#include <set>
#include <list>

#include "thread.h"

class ClientConnection;
class DataWriter;
class BackwardIndexBuilder;
class ImageProcessor;
class IndexMode;

using namespace std;


class Server : public Thread
{
public:
    Server(DataWriter *dataWriter, BackwardIndexBuilder *backwardIndexBuilder,
           ImageProcessor *imageProcessor,
           IndexMode *mode, ThreadManager *threadManager);
    virtual ~Server();
    void stop();
    void removeClient(ClientConnection *c);

private:
    void *run();

    int portNumber;

    set<ClientConnection *> clients;
    list<ClientConnection *> clientsToRemove;

    ThreadManager *threadManager;

    DataWriter *dataWriter;
    BackwardIndexBuilder *backwardIndexBuilder;
    ImageProcessor *imageProcessor;
    IndexMode *mode;

    int sock;
    int closeFd;
    int closeFdRead;
};

#endif // SERVER_H
