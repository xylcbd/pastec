#include <iostream>

#include "server.h"
#include "forwardindexbuilder.h"
#include "backwardindexbuilder.h"
#include "imagefeatureextractor.h"
#include "imagesearcher.h"
#include "indexmode.h"


using namespace std;

int main(int argc, char** argv)
{
    cout << "Pastec Index v0.0.1" << endl;

    if (argc < 3)
    {
        cout << "Usage :" << endl
             << "./ImageIndexer visualWords index" << endl;
        return 1;
    }

    ForwardIndexBuilder *fib = new ForwardIndexBuilder("forwardIndex.dat");
    ImageFeatureExtractor *ife = new ImageFeatureExtractor(string(argv[1]), string(argv[2]));
    BackwardIndexBuilder *bib = new BackwardIndexBuilder("forwardIndex.dat",
                                                         "backwardIndex.dat");
    ImageSearcher *is = new ImageSearcher("backwardIndex.dat", string(argv[1]),
                                          string(argv[2]));
    IndexMode mode;
    Server *s = new Server(fib, bib, ife, is, &mode);

    s->start();

    s->join();

    delete fib;
    delete bib;
    delete s;

    return 0;
}
