#ifndef SERVER_H
#define SERVER_H

#include <set>
#include <list>

#include "thread.h"

class ClientConnection;
class ForwardIndexBuilder;
class BackwardIndexBuilder;
class ImageFeatureExtractor;
class ImageSearcher;
class IndexMode;

using namespace std;


#define MAX_NB_CLIENTS 30


class Server : public Thread
{
public:
    Server(ForwardIndexBuilder *forwardIndexBuilder,
           BackwardIndexBuilder *backwardIndexBuilder,
           ImageFeatureExtractor *imageProcessor,
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
    unsigned i_curNbClients;

    ForwardIndexBuilder *forwardIndexBuilder;
    BackwardIndexBuilder *backwardIndexBuilder;
    ImageFeatureExtractor *imageProcessor;
    ImageSearcher *imageSearcher;
    IndexMode *mode;

    int sock;
    int closeFd;
    int closeFdRead;

    pthread_mutex_t clientsMutex;
};

#endif // SERVER_H
