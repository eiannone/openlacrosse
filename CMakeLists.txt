#
# Configuration
#

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.7)
PROJECT(openlacrosse)

# Local modules
SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules)

# Standard libraries
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

# Extra warnings
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

# Add address sanitizer to debug build
IF (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang" AND NOT ${CMAKE_CXX_COMPILER_VERSION} LESS "3.2")
        MESSAGE(STATUS "Compiling using address sanitizer")
        SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O1 -fsanitize=address -fno-omit-frame-pointer")
ENDIF()


#
# Dependencies
#

# Boost
FIND_PACKAGE(Boost REQUIRED COMPONENTS program_options date_time)
INCLUDE_DIRECTORIES(SYSTEM ${Boost_INCLUDE_DIR})

# Precompiled headers
FIND_PACKAGE(PCHSupport)
IF (PCHSupport_FOUND)
        MESSAGE(STATUS "Using precompiled headers")
        ADD_PCH(src/std_includes.hpp)
        ADD_PCH(src/boost_includes.hpp)
ENDIF (PCHSupport_FOUND)


#
# Common libraries
#

ADD_LIBRARY(auxiliary src/auxiliary.hpp src/auxiliary.cpp)
TARGET_LINK_LIBRARIES(auxiliary ${Boost_LIBRARIES})

ADD_LIBRARY(station src/station.hpp src/station.cpp)
TARGET_LINK_LIBRARIES(station ${Boost_LIBRARIES})

ADD_LIBRARY(serialinterface src/serialinterface.hpp src/serialinterface.cpp)
TARGET_LINK_LIBRARIES(serialinterface auxiliary)

TARGETS_USE_PCH(auxiliary station serialinterface)


#
# WS8610
#

ADD_LIBRARY(ws8610 src/ws8610.hpp src/ws8610.cpp)
TARGET_LINK_LIBRARIES(ws8610 auxiliary station serialinterface)

TARGETS_USE_PCH(ws8610)


#
# Executables
#

ADD_EXECUTABLE(lacrosse src/main.cpp)
TARGET_LINK_LIBRARIES(lacrosse ws8610)

TARGETS_USE_PCH(lacrosse)
