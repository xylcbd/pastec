#ifndef INDEXMODE_H
#define INDEXMODE_H

enum Modes
{
    NO_MODE = 0,
    BUILD_FORWARD_INDEX_MODE = 1,
    BUILD_BACKWARD_INDEX_MODE = 2,
    SEARCH_MODE = 3
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
