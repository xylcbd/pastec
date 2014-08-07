#ifndef REQUESTHANDLER_H
#define REQUESTHANDLER_H

#include <httpserver.h>

class FeatureExtractor;
class Searcher;
class Index;

using namespace std;

namespace Json {
    class Value;
}


class RequestHandler
{
public:
    RequestHandler(FeatureExtractor *featureExtractor,
                   Searcher *imageSearcher, Index *index);
    void handleRequest(ConnectionInfo &conInfo);

private:
    vector<string> parseURI(string uri);
    bool testURIWithPattern(vector<string> parsedURI, string p_pattern[]);
    string JsonToString(Json::Value data);
    Json::Value StringToJson(string str);

    FeatureExtractor *featureExtractor;
    Searcher *imageSearcher;
    Index *index;
};

#endif // REQUESTHANDLER_H
