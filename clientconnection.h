#ifndef CLIENTCONNECTION_H
#define CLIENTCONNECTION_H

#include "thread.h"
#include "forwardindexbuilder.h"
#include "backwardindexbuilder.h"


class Server;
class IndexMode;
class ImageFeatureExtractor;
class ImageSearcher;


#define MAXMSG 2*1024*1024

struct Buffer
{
    char data[MAXMSG];
    unsigned length;
};


class ClientConnection : public Thread
{
public:
    ClientConnection(int socketFd, ForwardIndexBuilder *forwardIndexBuilder,
                     BackwardIndexBuilder *backwardIndexBuilder,
                     ImageFeatureExtractor *imageFeatureExtractor,
                     ImageSearcher *imageSearcher,
                     IndexMode *mode, Server *server);
    virtual ~ClientConnection();
    void stop();
    bool sendReply(unsigned i_replyLen, char *p_reply);
    bool sendReply(char reply);

private:
    void *run();
    int readData();
    void parseMessages();
    bool closeCurrentMode();

    int socketFd;
    int closeFd;
    int closeFdRead;
    Buffer buf;
    ForwardIndexBuilder *forwardIndexBuilder;
    BackwardIndexBuilder *backwardIndexBuilder;
    ImageFeatureExtractor *imageFeatureExtractor;
    ImageSearcher *imageSearcher;
    IndexMode *mode;
    Server *server;
};

#endif // CLIENTCONNECTION_H
