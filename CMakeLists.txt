cmake_minimum_required(VERSION 3.14)

project("Eigen2Mat")

find_package(Eigen3 REQUIRED)
find_package(Matlab REQUIRED)

add_library(${PROJECT_NAME} INTERFACE)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

include(GNUInstallDirs)

target_include_directories(${PROJECT_NAME} INTERFACE
  $<BUILD_INTERFACE:${${PROJECT_NAME}_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(${PROJECT_NAME} INTERFACE
  Eigen3::Eigen
  Matlab::MatlabEngine
  Matlab::MatlabDataArray
)

target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_11)

add_subdirectory(example)