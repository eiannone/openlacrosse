#
# Configuration
#

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.7)
PROJECT(openlacrosse)

# Standard libraries
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

# Extra warnings
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

# Add address sanitizer to debug build
IF (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang" AND NOT ${CMAKE_CXX_COMPILER_VERSION} LESS "3.2")
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O1 -fsanitize=address -fno-omit-frame-pointer")
ENDIF()


#
# Dependencies
#

# Boost
FIND_PACKAGE(Boost COMPONENTS program_options date_time)
INCLUDE_DIRECTORIES(SYSTEM ${Boost_INCLUDE_DIR})


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
