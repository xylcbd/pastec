#include <unistd.h>
#include <sys/socket.h>

#include <poll.h>

#include <string.h>
#include <assert.h>

#include "clientconnection.h"
#include "dataMessages.h"
#include "server.h"
#include "indexmode.h"
#include "threadmanager.h"
#include "imageprocessor.h"


ClientConnection::ClientConnection(int socketFd, DataWriter *dataWriter,
                                   BackwardIndexBuilder *backwardIndexBuilder,
                                   ImageProcessor *imageProcessor,
                                   IndexMode *mode, Server *server,
                                   ThreadManager *threadManager)
    : socketFd(socketFd), dataWriter(dataWriter),
      backwardIndexBuilder(backwardIndexBuilder),
      imageProcessor(imageProcessor),
      mode(mode), server(server), threadManager(threadManager)
{
    /* Create a pipe. */
    int pipefd[2];
    assert(pipe(pipefd) == 0);
    closeFdRead = pipefd[0];
    closeFd = pipefd[1];
}


ClientConnection::~ClientConnection()
{
    /* Close the pipe. */
    close(closeFdRead);
    close(closeFd);
}



/**
 * @brief Run the client connection thread.
 * @return a pointer to return data.
 */
void *ClientConnection::run()
{
    /* Prepare the usage of poll */
    int timeout = 3 * 60 * 1000;
    struct pollfd fds[2];
    fds[0].fd = socketFd;
    fds[0].events = POLLIN;
    fds[1].fd = closeFdRead;
    fds[1].events = POLLIN;

    while (!b_mustStop)
    {
        int rc = poll(fds, 2, timeout);

        if (rc < 0)
        {
            cout << "poll() failed" << endl;
            break;
        }
        else if (rc == 0)
        {
            cout << "Connection timed out." << endl;
            b_mustStop = true;
            server->removeClient(this);
        }
        else if (rc > 0)
        {
            for (unsigned i = 0; i < 2; i++)
            {
                if (fds[i].revents == 0)
                    continue;

                if (fds[i].fd == closeFdRead) // The server is stoping.
                {
                    cout << "Client must stop." << endl;
                    b_mustStop = true;
                }
                else if (fds[i].fd == socketFd)
                {
                    if (readData() < 0)
                    {
                        /* Connection closed by the client. */
                        b_mustStop = true;
                        server->removeClient(this);
                    }
                }
                else
                {
                    // Should not happen.
                    assert(0);
                }
            }
        }
    }

    close(socketFd);

    return NULL;
}


void ClientConnection::stop()
{
    assert(write(closeFd, "1", 1) == 1);
    join();
}


/**
 * @brief Read data from a connected client.
 * @return the size of read data. -1 on error or connection closed.
 */
int ClientConnection::readData()
{
    char buffer[MAXMSG];
    int nBytes;

    nBytes = read(socketFd, buffer, MAXMSG);
    if (nBytes < 0)
    {
        cout << "Error on read." << endl;
        return -1;
    }
    else if (nBytes == 0)
    {
        cout << "Connection closed." << endl;
        return -1;
    }
    else
    {
        /* Data read. */
        if (buf.length + nBytes < MAXMSG)
        {
            memcpy(buf.data + buf.length, buffer, nBytes);
            buf.length += nBytes;
        }
        else
        {
            cout << "Server buffer full of data !" << endl;
            return -1;
        }

        parseMessages();

        return 0;
    }
}


void ClientConnection::parseMessages()
{
    char *p = buf.data;

    assert(buf.length > 0);

    while (buf.length > 0)
    {
        switch (buf.data[0])
        {
        case INIT_BUILD_FORWARD_INDEX:
        {
            if (!closeCurrentMode())
            {
                sendReply(ERROR_GENERIC);
                return;
            }
            mode->mode = BUILD_FORWARD_INDEX_MODE;
            threadManager->startThread(dataWriter);

            sendReply(OK);

            memmove(p, p + 1, buf.length - 1);
            buf.length -= 1;
            break;
        }
        case BUILD_BACKWARD_INDEX:
        {
            if (!closeCurrentMode())
            {
                sendReply(ERROR_GENERIC);
                return;
            }
            mode->mode = BUILD_BACKWARD_INDEX_MODE;
            threadManager->startThread(backwardIndexBuilder);

            sendReply(OK);

            memmove(p, p + 1, buf.length - 1);
            buf.length -= 1;
            break;
        }
        case INDEX:
        {
            if (mode->mode != BUILD_FORWARD_INDEX_MODE)
            {
                sendReply(WRONG_MODE);
                return;
            }

            if (buf.length < MSG_INDEX_LEN)
                return; // The complete message was not received.

            Hit newHit;
            newHit.i_wordId = *(u_int32_t *)(p + 1);
            newHit.i_imageId = *(u_int32_t *)(p + 5);
            newHit.i_angle = *(u_int16_t *)(p + 9);
            newHit.x = *(u_int16_t *)(p + 11);
            newHit.y = *(u_int16_t *)(p + 13);
            /* Check the current mode of the index. */
            if (mode->mode == BUILD_FORWARD_INDEX_MODE)
                dataWriter->queueHit(newHit);
            memmove(p, p + MSG_INDEX_LEN, buf.length - MSG_INDEX_LEN);
            buf.length -= MSG_INDEX_LEN;
            break;
        }
        case INDEX_IMAGE:
        {
            if (mode->mode != BUILD_FORWARD_INDEX_MODE)
            {
                sendReply(WRONG_MODE);
                return;
            }

            if (buf.length < MSG_INDEX_IMAGE_HEADER_LEN)
                return; // The complete message was not received.

            unsigned i_imageId = *(u_int32_t *)(p + 1);
            unsigned i_imageSize = *(u_int32_t *)(p + 5);
            unsigned i_msgSize = MSG_INDEX_IMAGE_HEADER_LEN + i_imageSize;

            // Test if the image is not too big.
            if (i_imageSize > 1024 * 1024)
            {
                sendReply(IMAGE_DATA_TOO_BIG);
                return;
            }

            // Test if we have received all the image data.
            if (buf.length < i_msgSize)
                return;

            bool b_ret = imageProcessor->processNewImage(i_imageId, i_imageSize, p + MSG_INDEX_IMAGE_HEADER_LEN);

            if (b_ret)
                sendReply(OK);
            else
                sendReply(IMAGE_NOT_INDEXED);

            memmove(p, p + i_msgSize, buf.length - i_msgSize);
            buf.length -= i_msgSize;
            break;
        }
        case PING:
        {
            sendReply(PONG);
            memmove(p, p + 1, buf.length - 1);
            buf.length -= 1;
            break;
        }
        case STOP:
        {
            cout << "Stop of the server asked." << endl;
            server->stop();
            memmove(p, p + 1, buf.length - 1);
            buf.length -= 1;
            break;
        }
        default:
        {
            cout << "Got garbage message?" << endl;
            memmove(p, p + 1, buf.length - 1);
            buf.length -= 1;
            break;
        }
        }
    }
}


bool ClientConnection::sendReply(char reply)
{
    char p_reply[] = {reply};
    return sendReply(sizeof(p_reply), p_reply);
}


bool ClientConnection::sendReply(unsigned i_replyLen, char *p_reply)
{
    unsigned i_nbBytesSent = 0;
    while (i_nbBytesSent < i_replyLen)
    {
        unsigned n = send(socketFd, p_reply, i_replyLen - i_nbBytesSent, 0);
        if (n <= 0)
            return false;
        i_nbBytesSent += n;
    }
    return true;
}


/**
 * @brief Close the current index mode.
 */
bool ClientConnection::closeCurrentMode()
{
    bool b_ret = true;

    switch (mode->mode)
    {
    case NO_MODE:
        break;
    case BUILD_FORWARD_INDEX_MODE:
        dataWriter->stop();
        break;
    case BUILD_BACKWARD_INDEX_MODE:
        if (!backwardIndexBuilder->hasFinished())
            b_ret = false;
        break;
    default:
        assert(0);
        break;
    }

    return b_ret;
}
