#ifndef INDEXMODE_H
#define INDEXMODE_H

enum Modes
{
    NO_MODE = 0,
    IMAGE_FEATURE_EXTRACTOR_MODE = 1,
    BUILD_FORWARD_INDEX_MODE = 2,
    BUILD_BACKWARD_INDEX_MODE = 3,
    SEARCH_MODE = 4
};


class IndexMode
{
    friend class ClientConnection;

public:
    IndexMode() : mode(NO_MODE) {};

private:
    Modes mode;
};


#endif // INDEXMODE_H
