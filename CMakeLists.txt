cmake_minimum_required(VERSION 2.6)
project(openlacrosse)

FIND_PACKAGE(Boost)
IF (Boost_FOUND)
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
    ADD_DEFINITIONS( "-DHAS_BOOST" )
ENDIF()

SET(CMAKE_CXX_FLAGS "-std=c++0x")

add_library(auxiliary src/auxiliary.h src/auxiliary.cpp)

add_library(serialinterface src/serialinterface.h src/serialinterface.cpp)
target_link_libraries(serialinterface auxiliary)

add_library(ws8610 src/ws8610.h src/ws8610.cpp)
target_link_libraries(ws8610 auxiliary serialinterface)

add_executable(main src/main.cpp)
target_link_libraries(main ws8610)
