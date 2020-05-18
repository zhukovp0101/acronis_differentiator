project(tests)

set(CMAKE_CXX_STANDARD 17)

if(CMAKE_CXX_COMPILER_ID MATCHES GNU)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(gtest)

enable_testing()

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_executable(DifferentiatorTests DifferentiatorTests.cpp)
target_link_libraries(DifferentiatorTests gtest gtest_main project_lib TexCaller)
add_test(DifferentiatorTests ${CMAKE_BINARY_DIR}/bin/Tests)

add_executable(VectorTests VectorTests.cpp Helper.cpp Helper.h)
target_link_libraries(VectorTests gtest gtest_main project_lib)
add_test(VectorTests ${CMAKE_BINARY_DIR}/bin/Tests)

add_executable(ListTests ListTests.cpp Helper.cpp Helper.h)
target_link_libraries(ListTests gtest gtest_main project_lib)
add_test(ListTests ${CMAKE_BINARY_DIR}/bin/Tests)

add_executable(MapTests MapTests.cpp Helper.cpp Helper.h)
target_link_libraries(MapTests gtest gtest_main project_lib)
add_test(MapTests ${CMAKE_BINARY_DIR}/bin/Tests)