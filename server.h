#ifndef SERVER_H
#define SERVER_H

#include <set>
#include <list>

#include "thread.h"

class ClientConnection;
class ORBFeatureExtractor;
class ImageSearcher;
class Index;

using namespace std;


#define MAX_NB_CLIENTS 30


class Server : public Thread
{
public:
    Server(ORBFeatureExtractor *imageProcessor,
           ImageSearcher *imageSearcher, Index *index);
    virtual ~Server();
    void stop();
    void removeClient(ClientConnection *c);

private:
    void *run();

    int portNumber;

    set<ClientConnection *> clients;
    list<ClientConnection *> clientsToRemove;
    unsigned i_curNbClients;

    ORBFeatureExtractor *imageProcessor;
    ImageSearcher *imageSearcher;
    Index *index;

    int sock;
    int closeFd;
    int closeFdRead;

    pthread_mutex_t clientsMutex;
};

#endif // SERVER_H
