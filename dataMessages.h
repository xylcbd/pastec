#ifndef MESSAGES_H
#define MESSAGES_H


enum Queries
{
    INDEX_IMAGE =                       0x00010000,
    SEARCH =                            0x00020000,
    PING =                              0x00030000,
    WRITE_INDEX =                       0x00040000,
    CLEAR_INDEX =                       0x00050000
};

enum Replies
{
    OK =                                0x10010000,
    ERROR_GENERIC =                     0x10020000,
    PONG =                              0x10030000,
    TOO_MANY_CLIENTS =                  0x10040000,

    IMAGE_DATA_TOO_BIG =                0x10050100,
    IMAGE_NOT_INDEXED =                 0x10050200,
    IMAGE_SIZE_TOO_BIG =                0x10050300,
    IMAGE_NOT_DECODED =                 0x10050400,
    IMAGE_SIZE_TOO_SMALL =              0x10050500,
};

#define MSG_INDEX_IMAGE_HEADER_LEN 12


#endif // MESSAGES_H
