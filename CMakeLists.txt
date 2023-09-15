cmake_minimum_required(VERSION 3.15)
project(Server LANGUAGES C CXX)

# 设置c++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_EXTENSIONS OFF)

#设置输出路径
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin/${PROJECT_NAME})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin/${PROJECT_NAME})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin/${PROJECT_NAME})

set(CMAKE_BUILD_TYPE DEBUG)

find_package(yaml-cpp REQUIRED)
# include_directories(${PROJECT_NAME} /usr/local/include/yaml-cpp)
# message(STATUS "Used config file: ${yaml-cpp_CONFIG}")

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp src/*.h src/*.hpp  vendor/HttpParser/*.cpp)
# set(SOURCES src/config/config.h src/config/config.cpp src/pch.h)
# add_executable(${PROJECT_NAME} ${SOURCES})
# precompile_headers(src/pch.h)
include_directories(${YAML_CPP_INCLUDE_DIR} src vendor)
link_libraries(yaml-cpp)
add_executable(origin_test tests/test_server.cpp ${SOURCES})

# target_precompile_headers(${PROJECT_NAME} PRIVATE src/pch.h)
# target_include_directories(${PROJECT_NAME} PUBLIC ${YAML_CPP_INCLUDE_DIR} src)
# target_link_libraries(${PROJECT_NAME}  yaml-cpp)