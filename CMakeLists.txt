cmake_minimum_required(VERSION 3.15)
project(db_ConnectionPool)
set(CMAKE_CXX_STANDARD 11)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(/usr/include/jsoncpp)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)


aux_source_directory(${PROJECT_SOURCE_DIR}/src SRC_LIST)

add_executable(
    db_ConnectionPool
    ${SRC_LIST}
)
