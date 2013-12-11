#include <iostream>

#include "server.h"
#include "datawriter.h"
#include "backwardindexbuilder.h"
#include "imageprocessor.h"
#include "indexmode.h"


using namespace std;

int main(int argc, char** argv)
{
    cout << "Pastec Index v0.0.1" << endl;

    if (argc < 2)
    {
        cout << "./ImageIndexer visualWords" << endl;
        return 1;
    }

    DataWriter *dw = new DataWriter("forwardIndex.dat");
    BackwardIndexBuilder *bib = new BackwardIndexBuilder("forwardIndex.dat",
                                                         "backwardIndex.dat");
    ImageProcessor *ip = new ImageProcessor(dw, string(argv[1]));
    IndexMode mode;
    Server *s = new Server(dw, bib, ip, &mode);

    s->start();

    s->join();

    delete dw;
    delete bib;
    delete s;

    return 0;
}
