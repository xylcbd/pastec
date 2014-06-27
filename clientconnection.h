#ifndef CLIENTCONNECTION_H
#define CLIENTCONNECTION_H

#include <sys/types.h>

#include "thread.h"


class Server;
class FeatureExtractor;
class ORBSearcher;
class Index;


#define MAXMSG 2*1024*1024

struct Buffer
{
    char data[MAXMSG];
    unsigned length;
};


class ClientConnection : public Thread
{
public:
    ClientConnection(int socketFd,
                     FeatureExtractor *imageFeatureExtractor,
                     ORBSearcher *imageSearcher, Index *index,
                     Server *server);
    virtual ~ClientConnection();
    void stop();
    bool sendReply(unsigned i_replyLen, char *p_reply);
    bool sendReply(u_int32_t reply);

private:
    void *run();
    int readData();
    void parseMessages();

    int socketFd;
    int closeFd;
    int closeFdRead;
    Buffer buf;
    FeatureExtractor *orbFeatureExtractor;
    ORBSearcher *imageSearcher;
    Index *index;
    Server *server;
};

#endif // CLIENTCONNECTION_H
