cmake_minimum_required(VERSION 3.9)
project(oslab_mrfs)

set(CMAKE_CXX_STANDARD 11)

add_executable(oslab_mrfs
        mrfs.cpp
        mrfs.h
        test.cpp)
