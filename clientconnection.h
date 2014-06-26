#ifndef CLIENTCONNECTION_H
#define CLIENTCONNECTION_H

#include <sys/types.h>

#include "thread.h"


class Server;
class ORBFeatureExtractor;
class ORBSearcher;
class ORBIndex;


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
                     ORBFeatureExtractor *imageFeatureExtractor,
                     ORBSearcher *imageSearcher, ORBIndex *index,
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
    ORBFeatureExtractor *orbFeatureExtractor;
    ORBSearcher *imageSearcher;
    ORBIndex *index;
    Server *server;
};

#endif // CLIENTCONNECTION_H
