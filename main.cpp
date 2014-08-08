#include <iostream>
#include <signal.h>

#include "httpserver.h"
#include "requesthandler.h"
#include "orb/orbfeatureextractor.h"
#include "orb/orbsearcher.h"
#include "orb/orbwordindex.h"


using namespace std;

HTTPServer *s;

void intHandler(int signum) {
    (void)signum;
    s->stop();
}


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
    s = new HTTPServer(rh, 4212);

    signal(SIGHUP, intHandler);
    signal(SIGINT, intHandler);

    s->run();

    cout << "Terminating Pastec." << endl;

    delete s;
    delete (ORBSearcher *)is;
    delete (ORBFeatureExtractor *)ife;
    delete (ORBIndex *)index;

    return 0;
}
