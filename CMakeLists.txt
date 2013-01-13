CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(openlacrosse)


#
# Dependencies
#

FIND_PACKAGE(Boost COMPONENTS program_options date_time)
IF (Boost_FOUND)
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
ENDIF()

SET(CMAKE_CXX_FLAGS "-std=c++0x")


#
# Common libraries
#

ADD_LIBRARY(auxiliary src/auxiliary.h src/auxiliary.cpp)
TARGET_LINK_LIBRARIES(auxiliary ${Boost_LIBRARIES})

ADD_LIBRARY(station src/station.h src/station.cpp)
TARGET_LINK_LIBRARIES(station ${Boost_LIBRARIES})

ADD_LIBRARY(serialinterface src/serialinterface.h src/serialinterface.cpp)
TARGET_LINK_LIBRARIES(serialinterface auxiliary)


#
# WS8610
#

ADD_LIBRARY(ws8610 src/ws8610.h src/ws8610.cpp)
TARGET_LINK_LIBRARIES(ws8610 auxiliary station serialinterface)


#
# Executables
#

ADD_EXECUTABLE(lacrosse src/main.cpp)
TARGET_LINK_LIBRARIES(lacrosse ws8610)
