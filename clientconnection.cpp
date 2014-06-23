#include <unistd.h>
#include <sys/socket.h>

#include <poll.h>

#include <string.h>
#include <assert.h>

#include "clientconnection.h"
#include "dataMessages.h"
#include "server.h"
#include "imagefeatureextractor.h"
#include "imagesearcher.h"


ClientConnection::ClientConnection(int socketFd,
                                   ImageFeatureExtractor *imageProcessor,
                                   ImageSearcher *imageSearcher, Index *index,
                                   Server *server)
    : socketFd(socketFd), imageFeatureExtractor(imageProcessor),
      imageSearcher(imageSearcher), index(index),
      server(server)
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
    int timeout = 86400 * 1000; // one day.
    struct pollfd fds[2];
    fds[0].fd = socketFd;
    fds[0].events = POLLIN;
    fds[1].fd = closeFdRead;
    fds[1].events = POLLIN;

    // Tell the client that the server is ready.
    sendReply(OK);

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

    while (buf.length >= 4)
    {
        u_int32_t *msg = (u_int32_t *)p;

        switch (*msg)
        {
        case INDEX_IMAGE:
        {
            if (buf.length < MSG_INDEX_IMAGE_HEADER_LEN)
                return; // The complete message was not received.

            unsigned i_imageId = *(u_int32_t *)(p + 4);
            unsigned i_imageSize = *(u_int32_t *)(p + 8);
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

            u_int32_t i_ret = imageFeatureExtractor->processNewImage(
                i_imageId, i_imageSize, p + MSG_INDEX_IMAGE_HEADER_LEN, this);

            sendReply(i_ret);

            memmove(p, p + i_msgSize, buf.length - i_msgSize);
            buf.length -= i_msgSize;
            break;
        }
        case REMOVE_IMAGE:
        {
            if (buf.length < MSG_REMOVE_IMAGE_LEN)
                return; // The complete message was not received.

            unsigned i_imageId = *(u_int32_t *)(p + 4);
            bool success = index->removeImage(i_imageId);
            if (success)
                sendReply(OK);
            else
                sendReply(ERROR_GENERIC);

            memmove(p, p + MSG_REMOVE_IMAGE_LEN, buf.length - MSG_REMOVE_IMAGE_LEN);
            buf.length -= MSG_REMOVE_IMAGE_LEN;
            break;
        }
        case WRITE_INDEX:
        {
            bool success = index->write();
            if (success)
                sendReply(OK);
            else
                sendReply(ERROR_GENERIC);

            memmove(p, p + 4, buf.length - 4);
            buf.length -= 4;
            break;
        }
        case CLEAR_INDEX:
        {
            bool success = index->clear();
            if (success)
                sendReply(OK);
            else
                sendReply(ERROR_GENERIC);

            memmove(p, p + 4, buf.length - 4);
            buf.length -= 4;
            break;
        }
        case PING:
        {
            sendReply(PONG);
            memmove(p, p + 4, buf.length - 4);
            buf.length -= 4;
            break;
        }
        case SEARCH:
        {
            /* Search request:
             * - 1 byte for the command code.
             * - 4 byte to give the size of the request image.
             * - X bytes for the image data.
             */

            if (buf.length < 8) // Not enough bytes to decode the msg length.
                return;

            u_int32_t i_imageSize = *(u_int32_t *)(p + 4);
            unsigned i_msgSize = 8 + i_imageSize;

            if (buf.length < i_msgSize) // The picture has not been entirely received.
                return;

            SearchRequest req;
            req.imageData.resize(i_imageSize);
            req.client = this;
            memcpy((void *)&req.imageData[0], p + 8, i_imageSize);
            imageSearcher->searchImage(req);

            assert(i_msgSize >= buf.length);
            memmove(p, p + i_msgSize, buf.length - i_msgSize);
            buf.length -= i_msgSize;
            break;
        }
        default:
        {
            cout << "Got garbage message?" << endl;
            memmove(p, p + 4, buf.length - 4);
            buf.length -= 4;
            break;
        }
        }
    }
}


/**
 * @brief Send a reply code
 * @param reply the reply code
 * @return true on success else false.
 */
bool ClientConnection::sendReply(u_int32_t reply)
{
    u_int32_t p_reply[] = {reply};
    return sendReply(sizeof(p_reply), (char *)p_reply);
}


/**
 * @brief Send a message to the client
 * @param i_replyLen the length of the message:
 * @param p_reply a pointer to the message
 * @return true on success else false.
 */
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
