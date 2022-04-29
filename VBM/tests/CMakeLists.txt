cmake_minimum_required(VERSION 3.5)
project(VBMTests LANGUAGES CXX)

find_package(Catch2 REQUIRED)

add_library(mockTarget STATIC unittests/target_main.cpp unittests/mock_rtc.hpp ../VBM/clock.hpp ../VBM/clock.cpp)
set_target_properties(mockTarget PROPERTIES SOVERSION 1)
set_target_properties(mockTarget PROPERTIES LINKER_LANGUAGE CXX)

add_executable(unittests unittests/main.cpp unittests/test_timer.cpp unittests/mock_rtc.hpp ../VBM/clock.hpp)

target_include_directories(unittests PRIVATE ../VBM)
target_include_directories(mockTarget PRIVATE ../VBM)

target_link_libraries(unittests PRIVATE mockTarget Catch2::Catch2)