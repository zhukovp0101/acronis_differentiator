cmake_minimum_required(VERSION 3.0)
project(Differentiator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(TexCaller)
include_directories(TexCaller)

add_library(project_lib STATIC src/Differenctiator/Differentiator.h src/Differenctiator/Differentiator.cpp
	src/Parser/Parser.h src/Parser/Parser.cpp src/String/String.h src/String/String.cpp src/Tree/Tree.h src/Tree/Tree.cpp
	src/UnorderedMap/UnorderedMap.h src/UnorderedMap/UnorderedMap.cpp src/UnorderedSet/UnorderedSet.h src/UnorderedSet/UnorderedSet.cpp src/Vector/Vector.h src/Vector/Vector.cpp src/List/List.cpp src/List/List.h)

include_directories(src/Differenctiator src/Parser src/String src/Tree src/UnorderedMap src/UnorderedSet src/Vector src/List)

add_subdirectory(stdin_stdout)
add_subdirectory(tests)