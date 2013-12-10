#ifndef CLIENTCONNECTION_H
#define CLIENTCONNECTION_H

#include "thread.h"
#include "datawriter.h"
#include "backwardindexbuilder.h"


class Server;
class IndexMode;
class ImageProcessor;


#define MAXMSG 2*1024*1024

struct Buffer
{
    char data[MAXMSG];
    unsigned length;
};


class ClientConnection : public Thread
{
public:
    ClientConnection(int socketFd, DataWriter *dataWriter,
                     BackwardIndexBuilder *backwardIndexBuilder,
                     ImageProcessor *imageProcessor,
                     IndexMode *mode, Server *server,
                     ThreadManager *threadManager);
    virtual ~ClientConnection();
    void stop();

private:
    void *run();
    int readData();
    void parseMessages();
    bool closeCurrentMode();

    bool sendReply(char reply);
    bool sendReply(unsigned i_replyLen, char *p_reply);

    int socketFd;
    int closeFd;
    int closeFdRead;
    Buffer buf;
    DataWriter *dataWriter;
    BackwardIndexBuilder *backwardIndexBuilder;
    ImageProcessor *imageProcessor;
    IndexMode *mode;
    Server *server;
    ThreadManager *threadManager;
};

#endif // CLIENTCONNECTION_H
