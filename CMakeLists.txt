 cmake_minimum_required(VERSION 3.6)
 project(LSMTree)
 set(CMAKE_CXX_STANDARD 17)
 set(CMAKE_CXX_STANDARD_REQUIRED ON)
 set(CMAKE_VERBOSE_MAKEFILE ON)

 set(SOURCE_FILES test.cpp)
 add_executable(main ${SOURCE_FILES})

 target_link_libraries(main stdc++fs)
 target_link_libraries(main c++experimental)
