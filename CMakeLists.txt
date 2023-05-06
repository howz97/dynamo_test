# CMake 最低版本号要求
cmake_minimum_required (VERSION 3.2)

# 项目信息
project (test-dynamodb)

set(CMAKE_CXX_STANDARD 17)

aux_source_directory(. DIR_SRCS)

find_package(AWSSDK REQUIRED COMPONENTS dynamodb)
find_package(ZLIB)

add_executable(dynamo_test ${DIR_SRCS})

target_link_libraries(dynamo_test ${AWSSDK_LINK_LIBRARIES})
target_link_libraries(dynamo_test ZLIB::ZLIB)