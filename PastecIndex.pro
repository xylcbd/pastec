TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    server.cpp \
    datawriter.cpp \
    clientconnection.cpp \
    backwardindexbuilder.cpp \
    imageprocessor.cpp \
    imagesearcher.cpp \
    backwardindexreader.cpp \
    imagereranker.cpp

HEADERS += \
    server.h \
    thread.h \
    datawriter.h \
    dataMessages.h \
    clientconnection.h \
    backwardindexbuilder.h \
    indexmode.h \
    imageprocessor.h \
    imagesearcher.h \
    hit.h \
    backwardindexreader.h \
    searchResult.h \
    imagereranker.h

LIBS += -pthread \
        -L/home/magsoft/trunks/opencv-2.4.6.1/build/lib \
        -lopencv_core \
        -lopencv_highgui \
        -lopencv_imgproc \
        -lopencv_flann \
        -lopencv_nonfree \
        -lopencv_features2d \
        -lopencv_calib3d \
        -lpq \
        -lcurl

INCLUDEPATH += /home/magsoft/trunks/opencv-2.4.6.1/modules/core/include \
            /home/magsoft/trunks/opencv-2.4.6.1/modules/features2d/include \
            /home/magsoft/trunks/opencv-2.4.6.1/modules/highgui/include \
            /home/magsoft/trunks/opencv-2.4.6.1/modules/imgproc/include \
            /home/magsoft/trunks/opencv-2.4.6.1/modules/flann/include \
            /home/magsoft/trunks/opencv-2.4.6.1/modules/nonfree/include \
            /home/magsoft/trunks/opencv-2.4.6.1/modules/calib3d/include
