CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(openlacrosse)

FIND_PACKAGE(Boost)
IF (Boost_FOUND)
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
    ADD_DEFINITIONS( "-DHAS_BOOST" )
ENDIF()

SET(CMAKE_CXX_FLAGS "-std=c++0x")

ADD_LIBRARY(auxiliary src/auxiliary.h src/auxiliary.cpp)

ADD_LIBRARY(serialinterface src/serialinterface.h src/serialinterface.cpp)
TARGET_LINK_LIBRARIES(serialinterface auxiliary)

ADD_LIBRARY(ws8610 src/ws8610.h src/ws8610.cpp)
TARGET_LINK_LIBRARIES(ws8610 auxiliary serialinterface)

ADD_EXECUTABLE(main src/main.cpp)
TARGET_LINK_LIBRARIES(main ws8610)
