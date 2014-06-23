#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <poll.h>

#include <assert.h>
#include <string.h>

#include <iostream>

#include "server.h"
#include "clientconnection.h"
#include "dataMessages.h"


Server::Server(ImageFeatureExtractor *imageProcessor,
               ImageSearcher *imageSearcher, Index *index)
    : portNumber(4212),
      i_curNbClients(0),
      imageProcessor(imageProcessor),
      imageSearcher(imageSearcher),
      index(index)
{
    /* Create a pipe. */
    int pipefd[2];
    assert(pipe(pipefd) == 0);
    closeFdRead = pipefd[0];
    closeFd = pipefd[1];

    pthread_mutex_init(&clientsMutex, NULL);
}


Server::~Server()
{
    pthread_mutex_destroy(&clientsMutex);

    /* Close the pipe. */
    close(closeFdRead);
    close(closeFd);
}


/**
 * @brief Run the server thread.
 * @return a pointer to return data.
 */
void *Server::run()
{
    cout << "Starting server." << endl;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
       cout << "Error on opening socket" << endl;
       return NULL;
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portNumber);

    if (bind(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        cout << "Error on binding" << endl;
        return NULL;
    }

    /* Listen for new connections. */
    listen(sock, 5);

    /* Prepare the usage of poll */
    int timeout = 3 * 60 * 1000;
    struct pollfd fds[2];
    fds[0].fd = sock;
    fds[0].events = POLLIN;
    fds[1].fd = closeFdRead;
    fds[1].events = POLLIN;

    cout << "Ready to accept querries." << endl;

    while (!b_mustStop)
    {
        int rc = poll(fds, 2, timeout);

        if (rc < 0)
        {
            cout << "poll() failed" << endl;
            break;
        }
        else if (rc > 0)
        {
            for (unsigned i = 0; i < 2; i++)
            {
                if (fds[i].revents == 0)
                    continue;

                if (fds[i].fd == closeFdRead)
                {
                    cout << "Server must stop." << endl;
                    b_mustStop = true;
                }
                else if (fds[i].fd == sock)
                {
                    /* Connection request on original socket.
                     * First, free the closed connection clients. */
                    for (list<ClientConnection *>::iterator it = clientsToRemove.begin();
                         it != clientsToRemove.end(); ++it)
                    {
                        cout << "Freeing one connection." << endl;
                        (*it)->stop();
                        delete *it;
                        set<ClientConnection *>::iterator itToRemove = clients.find(*it);
                        assert(itToRemove != clients.end());
                        clients.erase(itToRemove);
                    }
                    clientsToRemove.clear();

                    /* Handle the connection request. */
                    struct sockaddr_in clientname;
                    socklen_t size = sizeof(clientname);
                    int newFd = accept(sock, (struct sockaddr *)&clientname, &size);
                    if (newFd < 0)
                        cout << "Error on accept." << endl;
                    cout << "Server: connect from host " << inet_ntoa(clientname.sin_addr)
                         << ", port " << ntohs(clientname.sin_port) << "." << endl;

                    pthread_mutex_lock(&clientsMutex);
                    bool b_acceptClient = i_curNbClients < MAX_NB_CLIENTS ? true : false;
                    pthread_mutex_unlock(&clientsMutex);

                    if (b_acceptClient)
                    {
                        ClientConnection *c = new ClientConnection(newFd,
                                                                   imageProcessor,
                                                                   imageSearcher,
                                                                   index, this);
                        c->start();

                        pthread_mutex_lock(&clientsMutex);
                        i_curNbClients++;
                        clients.insert(c);
                        pthread_mutex_unlock(&clientsMutex);
                    }
                    else
                    {
                        u_int32_t p_msg[] = {TOO_MANY_CLIENTS};
                        send(newFd, p_msg, sizeof(p_msg), 0);
                        close(newFd);
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

    // Close all the sockets.
    cout << "Closing all the connections." << endl;
    close(sock);
    for (set<ClientConnection *>::iterator it = clients.begin();
         it != clients.end(); ++it)
    {
        (*it)->stop();
        delete *it;
    }

    return NULL;
}


/**
 * @brief Stop the server.
 */
void Server::stop()
{
    assert(write(closeFd, "1", 1) == 1);
}


/**
 * @brief Add a client in the list of clients to remove
 * @param c the client to remove.
 */
void Server::removeClient(ClientConnection *c)
{
    pthread_mutex_lock(&clientsMutex);
    i_curNbClients--;
    clientsToRemove.push_back(c);
    pthread_mutex_unlock(&clientsMutex);
}
