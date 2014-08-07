#include <iostream>

#include "server.h"
#include "httpserver.h"
#include "requesthandler.h"
#include "orb/orbfeatureextractor.h"
#include "orb/orbsearcher.h"
#include "orb/orbwordindex.h"


using namespace std;

int main(int argc, char** argv)
{
    cout << "Pastec Index v0.0.1" << endl;

    if (argc < 2)
    {
        cout << "Usage :" << endl
             << "./ImageIndexer visualWordList" << endl;
        return 1;
    }

    Index *index = new ORBIndex();
    ORBWordIndex *wordIndex = new ORBWordIndex(string(argv[1]));
    FeatureExtractor *ife = new ORBFeatureExtractor((ORBIndex *)index, wordIndex);
    Searcher *is = new ORBSearcher((ORBIndex *)index, wordIndex);
    RequestHandler *rh = new RequestHandler(ife, is, index);
    Server *s = new Server(ife, is, index);
    HTTPServer *s2 = new HTTPServer(rh);

    s2->start();
    s->start();

    s->join();

    delete s2;
    delete s;
    delete (ORBSearcher *)is;
    delete (ORBFeatureExtractor *)ife;
    delete (ORBIndex *)index;

    return 0;
}
