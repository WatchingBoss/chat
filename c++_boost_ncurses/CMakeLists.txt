cmake_minimum_required (VERSION 3.6)

set( SERVER "server" )
set( CLIENT "client" )
if(!MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -Wpedantic -Wno-long-long \
-Wno-variadic-macros -fexceptions")
endif()

# BOOST
set( BOOST_INC "/usr/local/include" )
set( BOOST_LIB "/usr/local/lib/boost" )
add_library( Boost INTERFACE )
target_link_libraries( Boost INTERFACE
  ${BOOST_LIB}/libboost_system.a
  ${BOOST_LIB}/libboost_thread.a
  -lpthread)
target_include_directories( Boost INTERFACE ${BOOST_INC} )
# END BOOST

file( GLOB COMMON_S_H ${PROJECT_SOURCE_DIR}/common.hpp ${PROJECT_SOURCE_DIR}/common.cpp)

file( GLOB SERVER_SOURCES ${PROJECT_SOURCE_DIR}/server/*cpp )
file( GLOB SERVER_HEADERS ${PROJECT_SOURCE_DIR}/server/*hpp )
file( GLOB CLIENT_SOURCES ${PROJECT_SOURCE_DIR}/client/*cpp )
file( GLOB CLIENT_HEADERS ${PROJECT_SOURCE_DIR}/client/*hpp )

add_executable( ${SERVER} ${SERVER_SOURCES} ${SERVER_HEADERS} ${COMMON_S_H})
target_link_libraries( ${SERVER} Boost )

add_executable( ${CLIENT} ${CLIENT_SOURCES} ${CLIENT_HEADERS} ${COMMON_S_H})
target_link_libraries( ${CLIENT} Boost )
