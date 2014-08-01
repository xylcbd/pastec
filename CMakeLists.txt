cmake_minimum_required(VERSION 2.6)
project(PastecIndex)

include_directories(. orb)

set(SOURCES      main.cpp
                 server.cpp
                 clientconnection.cpp
                 imagereranker.cpp
                 imagererankerransac.cpp
                 imageloader.cpp
                 orb/orbfeatureextractor.cpp
                 orb/orbindex.cpp
                 orb/orbsearcher.cpp
                 orb/orbwordindex.cpp)

set(HEADERS      server.h
                 thread.h
                 dataMessages.h
                 clientconnection.h
                 hit.h
                 searchResult.h
                 imagereranker.h
                 backwardindexreaderaccess.h
                 imageloader.h
                 index.h
                 featureextractor.h
                 orb/orbfeatureextractor.h
                 orb/orbindex.h
                 orb/orbsearcher.h
                 orb/orbwordindex.h
                 searcher.h)

find_package(Threads REQUIRED)
find_package(OpenCV REQUIRED)

add_executable(PastecIndex ${SOURCES} ${HEADERS})

target_link_libraries(PastecIndex ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(PastecIndex ${OpenCV_LIBS})

