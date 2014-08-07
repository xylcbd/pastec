#include <iostream>

#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <string.h>

#include "httpserver.h"
#include "dataMessages.h"
#include "requesthandler.h"


HTTPServer::HTTPServer(RequestHandler *requestHandler)
    : daemon(NULL), requestHandler(requestHandler)
{ }


#define PORT            4213
#define POSTBUFFERSIZE  512
#define MAXCLIENTS      2


int HTTPServer::start()
{
    daemon = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION, PORT, NULL, NULL,
                              &answerToConnection, this,
                              MHD_OPTION_NOTIFY_COMPLETED, requestCompleted,
                              NULL, MHD_OPTION_END);
    if (daemon == NULL)
        return ERROR_GENERIC;

    return OK;
}


int HTTPServer::stop()
{
    MHD_stop_daemon (daemon);
    return OK;
}


int HTTPServer::sendAnswer(struct MHD_Connection *connection, ConnectionInfo &conInfo)
{
    int ret;
    struct MHD_Response *response;

    const char *buffer = conInfo.answerString.c_str();

    response = MHD_create_response_from_buffer(strlen(buffer),
                                               (void *)buffer,
                                               MHD_RESPMEM_MUST_COPY);
    if (!response)
        return MHD_NO;

    ret = MHD_queue_response(connection, conInfo.answerCode, response);
    MHD_destroy_response(response);

    return ret;
}


void HTTPServer::requestCompleted(void *cls, MHD_Connection *connection,
                                  void **conCls, enum MHD_RequestTerminationCode toe)
{
    (void)cls, (void)connection; (void)toe;
    ConnectionInfo *conInfo = (ConnectionInfo *)*conCls;

    if (conInfo == NULL)
        return;

    if (conInfo->connectionType == POST
        && conInfo->postprocessor != NULL)
        MHD_destroy_post_processor(conInfo->postprocessor);

    delete conInfo;
    *conCls = NULL;
}


int HTTPServer::answerToConnection(void *cls, MHD_Connection *connection,
                                   const char *url, const char *method,
                                   const char *version, const char *upload_data,
                                   size_t *upload_data_size, void **conCls)
{
    (void)version;
    HTTPServer *s = (HTTPServer *)cls;
    ConnectionInfo *conInfo;

    if (*conCls == NULL)
    {
        conInfo = new ConnectionInfo();
        if (conInfo == NULL)
            return MHD_NO;

        if (strcmp(method, "POST") == 0)
            conInfo->connectionType = POST;
        else if (strcmp(method, "GET") == 0)
            conInfo->connectionType = GET;
        else if (strcmp(method, "DELETE") == 0)
            conInfo->connectionType = DELETE;
        else if (strcmp(method, "PUT") == 0)
            conInfo->connectionType = PUT;

        conInfo->url = string(url);

        *conCls = (void *) conInfo;

        return MHD_YES;
    }

    conInfo = (ConnectionInfo *)*conCls;

    if (conInfo->connectionType == GET
        || conInfo->connectionType == DELETE)
        s->requestHandler->handleRequest(*conInfo);
    else if (conInfo->connectionType == POST
             || conInfo->connectionType == PUT)
    {
        if (*upload_data_size != 0)
        {
            conInfo->uploadedData.insert(conInfo->uploadedData.end(),
                upload_data, upload_data + *upload_data_size);
            *upload_data_size = 0;

            return MHD_YES;
        }
        else
            s->requestHandler->handleRequest(*conInfo);
    }

    return sendAnswer(connection, *conInfo);
}


