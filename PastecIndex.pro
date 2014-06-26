TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    server.cpp \
    clientconnection.cpp \
    imagesearcher.cpp \
    imagereranker.cpp \
    imagefeatureextractor.cpp \
    index.cpp \
    wordindex.cpp \
    imagererankerransac.cpp \
    imageloader.cpp

HEADERS += \
    server.h \
    thread.h \
    dataMessages.h \
    clientconnection.h \
    imagesearcher.h \
    hit.h \
    searchResult.h \
    imagereranker.h \
    backwardindexreaderaccess.h \
    imagefeatureextractor.h \
    index.h \
    wordindex.h \
    imageloader.h

LIBS += /home/magsoft/trunks/opencv-2.4.6.1/build/lib/libopencv_calib3d.a \
        /home/magsoft/trunks/opencv-2.4.6.1/build/lib/libopencv_features2d.a \
        /home/magsoft/trunks/opencv-2.4.6.1/build/lib/libopencv_imgproc.a \
        /home/magsoft/trunks/opencv-2.4.6.1/build/lib/libopencv_highgui.a \
        /home/magsoft/trunks/opencv-2.4.6.1/build/3rdparty/lib/libIlmImf.a \
        /home/magsoft/trunks/opencv-2.4.6.1/build/lib/libopencv_flann.a \
        /home/magsoft/trunks/opencv-2.4.6.1/build/3rdparty/lib/liblibjasper.a \
        /home/magsoft/trunks/opencv-2.4.6.1/build/lib/libopencv_core.a \
        -pthread \
        -lpq \
        -ljpeg -lpng -ltiff \
        -lz

INCLUDEPATH += /home/magsoft/trunks/opencv-2.4.6.1/modules/core/include \
            /home/magsoft/trunks/opencv-2.4.6.1/modules/features2d/include \
            /home/magsoft/trunks/opencv-2.4.6.1/modules/highgui/include \
            /home/magsoft/trunks/opencv-2.4.6.1/modules/imgproc/include \
            /home/magsoft/trunks/opencv-2.4.6.1/modules/flann/include \
            /home/magsoft/trunks/opencv-2.4.6.1/modules/calib3d/include
