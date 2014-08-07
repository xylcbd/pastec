#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <vector>
#include <string>
#include <microhttpd.h>

using namespace std;

class RequestHandler;
struct ConnectionInfo;


#define GET             0
#define POST            1
#define DELETE          2
#define PUT             3

class HTTPServer
{
public:
    HTTPServer(RequestHandler *requestHandler);
    int start();
    int stop();

private:
    static int answerToConnection(void *cls, MHD_Connection *connection,
                                  const char *url, const char *method,
                                  const char *version, const char *upload_data,
                                  size_t *upload_data_size, void **con_cls);
    static void requestCompleted(void *cls, MHD_Connection *connection,
                                 void **con_cls, MHD_RequestTerminationCode toe);
    static int sendAnswer(struct MHD_Connection *connection, ConnectionInfo &conInfo);

    MHD_Daemon *daemon;
    RequestHandler *requestHandler;
};


struct ConnectionInfo
{
    ConnectionInfo() : b_receptionError(false) {}

    int connectionType;
    string url;
    bool b_receptionError;
    struct MHD_PostProcessor *postprocessor;
    string answerString;
    int answerCode;

    vector<char> uploadedData;
};

#endif // HTTPSERVER_H
