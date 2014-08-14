/*****************************************************************************
 * Copyright (C) 2014 Visualink
 *
 * Authors: Adrien Maglo <adrien@visualink.io>
 *
 * This file is part of Pastec.
 *
 * Pastec is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * Pastec is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Pastec.  If not, see <http://www.gnu.org/licenses/>.
 *****************************************************************************/

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
