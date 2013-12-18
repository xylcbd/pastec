#ifndef SERVER_H
#define SERVER_H

#include <set>
#include <list>

#include "thread.h"

class ClientConnection;
class DataWriter;
class BackwardIndexBuilder;
class ImageProcessor;
class ImageSearcher;
class IndexMode;

using namespace std;


class Server : public Thread
{
public:
    Server(DataWriter *dataWriter,
           BackwardIndexBuilder *backwardIndexBuilder,
           ImageProcessor *imageProcessor,
           ImageSearcher *imageSearcher,
           IndexMode *mode);
    virtual ~Server();
    void stop();
    void removeClient(ClientConnection *c);

private:
    void *run();

    int portNumber;

    set<ClientConnection *> clients;
    list<ClientConnection *> clientsToRemove;

    DataWriter *dataWriter;
    BackwardIndexBuilder *backwardIndexBuilder;
    ImageProcessor *imageProcessor;
    ImageSearcher *imageSearcher;
    IndexMode *mode;

    int sock;
    int closeFd;
    int closeFdRead;

    pthread_mutex_t clientsMutex;
};

#endif // SERVER_H
