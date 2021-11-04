cmake_minimum_required(VERSION 3.0)

project(VodServices)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT DEFINED MAGIC)
    message(FATAL_ERROR "Please Add The Directory Path Of The Magic Library!!!")
endif()

include_directories(
    ${MAGIC}
    ${MAGIC}/Include
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/Include
    ${MAGIC}/ThirdParty/Asio/Include
    ${MAGIC}/ThirdParty/RapidJSON/Include
)
link_directories(
    ${MAGIC}/Lib
)
link_libraries(Magic pqxx)
include(${MAGIC}/Magic.cmake)
include_directories(${CMAKE_BINARY_DIR})

set(MODULES
    ${MAGIC}/Modules/Magic.magic
    ${MAGIC}/Modules/WebServer.magic
    ${PROJECT_SOURCE_DIR}/Modules/VodServices.magic
)

set(SOURCES
    Source/Main.cpp
    Source/Servlet/AddMovieServlet.cpp
    Source/Servlet/ResourceServlet.cpp
    Source/Servlet/MovieListServlet.cpp
    Source/Servlet/MovieMenuServlet.cpp
    Source/Servlet/MovieDetailsServlet.cpp
)

add_custom_target(Gen ALL)
add_custom_command(
    TARGET Gen
    COMMAND ${MAGIC}/Bin/Gen ${MODULES} VodServices
)

add_executable(VodServices ${SOURCES})