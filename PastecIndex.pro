TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    server.cpp \
    clientconnection.cpp \
    backwardindexbuilder.cpp \
    imagesearcher.cpp \
    backwardindexreader.cpp \
    imagereranker.cpp \
    imagefeatureextractor.cpp \
    forwardindexbuilder.cpp

HEADERS += \
    server.h \
    thread.h \
    dataMessages.h \
    clientconnection.h \
    backwardindexbuilder.h \
    indexmode.h \
    imagesearcher.h \
    hit.h \
    backwardindexreader.h \
    searchResult.h \
    imagereranker.h \
    backwardindexreaderaccess.h \
    imagefeatureextractor.h \
    forwardindexbuilder.h

LIBS += -pthread \
        -L/home/magsoft/trunks/opencv-2.4.6.1/build/lib \
        -lopencv_core \
        -lopencv_highgui \
        -lopencv_imgproc \
        -lopencv_flann \
        -lopencv_nonfree \
        -lopencv_features2d \
        -lopencv_calib3d \
        -lpq

INCLUDEPATH += /home/magsoft/trunks/opencv-2.4.6.1/modules/core/include \
            /home/magsoft/trunks/opencv-2.4.6.1/modules/features2d/include \
            /home/magsoft/trunks/opencv-2.4.6.1/modules/highgui/include \
            /home/magsoft/trunks/opencv-2.4.6.1/modules/imgproc/include \
            /home/magsoft/trunks/opencv-2.4.6.1/modules/flann/include \
            /home/magsoft/trunks/opencv-2.4.6.1/modules/nonfree/include \
            /home/magsoft/trunks/opencv-2.4.6.1/modules/calib3d/include
