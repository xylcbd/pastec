#ifndef SEARCHER_H
#define SEARCHER_H

#include <sys/types.h>
#include <vector>

using namespace std;

class ClientConnection;


struct SearchRequest
{
    vector<char> imageData;
    ClientConnection *client;
};


class Searcher
{
public:
    virtual u_int32_t searchImage(SearchRequest request) = 0;
};

#endif // SEARCHER_H
