TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    server.cpp \
    clientconnection.cpp \
    imagereranker.cpp \
    imagererankerransac.cpp \
    imageloader.cpp \
    orb/orbfeatureextractor.cpp \
    orb/orbindex.cpp \
    orb/orbsearcher.cpp \
    orb/orbwordindex.cpp \
    httpserver.cpp \
    requesthandler.cpp

HEADERS += \
    server.h \
    thread.h \
    dataMessages.h \
    clientconnection.h \
    hit.h \
    searchResult.h \
    imagereranker.h \
    backwardindexreaderaccess.h \
    imageloader.h \
    index.h \
    featureextractor.h \
    orb/orbfeatureextractor.h \
    orb/orbindex.h \
    orb/orbsearcher.h \
    orb/orbwordindex.h \
    searcher.h \
    httpserver.h \
    requesthandler.h

LIBS += -pthread \
        -L/home/magsoft/trunks/opencv-2.4.6.1/build/lib \
        -lopencv_core \
        -lopencv_highgui \
        -lopencv_imgproc \
        -lopencv_flann \
        -lopencv_features2d \
        -lopencv_calib3d \
        -pthread \
        -lpq \
        -ljpeg -lpng -ltiff \
        -lmicrohttpd \
        -ljsoncpp

INCLUDEPATH += /home/magsoft/trunks/opencv-2.4.6.1/modules/core/include \
            /home/magsoft/trunks/opencv-2.4.6.1/modules/features2d/include \
            /home/magsoft/trunks/opencv-2.4.6.1/modules/highgui/include \
            /home/magsoft/trunks/opencv-2.4.6.1/modules/imgproc/include \
            /home/magsoft/trunks/opencv-2.4.6.1/modules/flann/include \
            /home/magsoft/trunks/opencv-2.4.6.1/modules/calib3d/include
