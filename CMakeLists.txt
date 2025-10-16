cmake_minimum_required(VERSION 3.20)

project(sensors_read LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

set(NAVIO2_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Navio2")
set(FETCHCONTENT_UPDATES_DISCONNECTED YES)
FetchContent_Declare(
  navio2
  GIT_REPOSITORY https://github.com/emlid/Navio2.git
  GIT_TAG master
  GIT_PROGRESS TRUE)
FetchContent_GetProperties(navio2)
if(NOT navio2_POPULATED)
  FetchContent_Populate(navio2)
endif()

set(NAVIO2_ROOT "${navio2_SOURCE_DIR}/C++/Navio")

file(GLOB NAVIO2_COMMON_SOURCES CONFIGURE_DEPENDS "${NAVIO2_ROOT}/Common/*.cpp")
file(GLOB NAVIO2_NAVIO2_SOURCES CONFIGURE_DEPENDS "${NAVIO2_ROOT}/Navio2/*.cpp")

add_library(navio2_drivers STATIC ${NAVIO2_COMMON_SOURCES}
                                  ${NAVIO2_NAVIO2_SOURCES})

target_include_directories(navio2_drivers PUBLIC "${NAVIO2_ROOT}")

target_compile_features(navio2_drivers PUBLIC cxx_std_23)

file(GLOB APP_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_executable(sensors_read_app ${APP_SOURCES})

target_include_directories(sensors_read_app
                           PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/incl")

target_link_libraries(sensors_read_app PRIVATE navio2_drivers)

set_target_properties(sensors_read_app PROPERTIES OUTPUT_NAME sensors_read)
