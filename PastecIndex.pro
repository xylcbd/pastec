TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    server.cpp \
    threadmanager.cpp \
    datawriter.cpp \
    clientconnection.cpp \
    backwardindexbuilder.cpp \
    imageprocessor.cpp

HEADERS += \
    server.h \
    threadmanager.h \
    thread.h \
    datawriter.h \
    dataMessages.h \
    clientconnection.h \
    backwardindexbuilder.h \
    indexmode.h \
    imageprocessor.h

LIBS += -pthread \
        -L/home/magsoft/trunks/opencv-2.4.6.1/build/lib \
        -lopencv_core \
        -lopencv_highgui \
        -lopencv_imgproc \
        -lopencv_flann \
        -lopencv_nonfree \
        -lopencv_features2d \
        -lpq \
        -lcurl

INCLUDEPATH += /home/magsoft/trunks/opencv-2.4.6.1/modules/core/include \
            /home/magsoft/trunks/opencv-2.4.6.1/modules/features2d/include \
            /home/magsoft/trunks/opencv-2.4.6.1/modules/highgui/include \
            /home/magsoft/trunks/opencv-2.4.6.1/modules/imgproc/include \
            /home/magsoft/trunks/opencv-2.4.6.1/modules/flann/include \
            /home/magsoft/trunks/opencv-2.4.6.1/modules/nonfree/include
