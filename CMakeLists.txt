cmake_minimum_required(VERSION 3.16)
project(sensors_read LANGUAGES CXX)

set(FETCHCONTENT_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/.external)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)

set(NAVIO2_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Navio2")
set(FETCHCONTENT_UPDATES_DISCONNECTED YES)
FetchContent_Declare(
  navio2
  GIT_REPOSITORY https://github.com/emlid/Navio2.git
  GIT_TAG master
  GIT_PROGRESS TRUE)
FetchContent_MakeAvailable(navio2)

set(NAVIO2_ROOT "${navio2_SOURCE_DIR}/C++/Navio")

file(GLOB NAVIO2_COMMON_SOURCES CONFIGURE_DEPENDS "${NAVIO2_ROOT}/Common/*.cpp")
file(GLOB NAVIO2_NAVIO2_SOURCES CONFIGURE_DEPENDS "${NAVIO2_ROOT}/Navio2/*.cpp")

add_library(navio2_drivers STATIC ${NAVIO2_COMMON_SOURCES}
                                  ${NAVIO2_NAVIO2_SOURCES})

target_include_directories(navio2_drivers PUBLIC "${NAVIO2_ROOT}")

target_compile_features(navio2_drivers PUBLIC cxx_std_20)

file(GLOB APP_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_executable(sensors_read ${APP_SOURCES})

target_include_directories(sensors_read
                           PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/incl")

target_compile_definitions(sensors_read PUBLIC ZENOHCXX_ZENOHC)
target_link_libraries(sensors_read PRIVATE navio2_drivers zenohc)
set_property(TARGET sensors_read PROPERTY LANGUAGE CXX)

add_executable(sensors_read_test test/subscriber.cpp)

target_include_directories(sensors_read_test
                           PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/incl")

target_link_libraries(sensors_read_test PRIVATE zenohc)
target_compile_definitions(sensors_read_test PUBLIC ZENOHCXX_ZENOHC)
