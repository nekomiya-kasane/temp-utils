cmake_minimum_required (VERSION 3.28)

project ("ustring")

include(D:/scoop/apps/vcpkg/2024.12.16/scripts/buildsystems/vcpkg.cmake)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#add_subdirectory(thirdparty/icu-cmake)
set(ICU_INCLUDE_DIR )
find_package(ICU REQUIRED COMPONENTS uc in i18n data)
find_package(TBB REQUIRED)
find_package(GTest REQUIRED)
find_package(benchmark REQUIRED)
#find_package(Hyperscan REQUIRED)

add_compile_options(/utf-8 /fsanitize=address)

link_libraries(ICU::uc ICU::i18n ICU::data ICU::in)# TBB::tbb)
link_libraries(TBB::tbb)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(GoogleTest)
include_directories(.)

add_library(ustring
    ustring.cpp
    ustring.h
    ustring.natvis
    inline_first_storage.h
    signal.h
)

add_executable(ustring_test
    ustring_construction_test.cpp
    ustring_iterator_test.cpp
    ustring_visit_test.cpp
    ustring_modification_test.cpp
    ustring_search_test.cpp
    ustring_transform_test.cpp
    signal_test.cpp
)

target_link_libraries(ustring_test
    PRIVATE
    ustring
    GTest::gtest
    GTest::gtest_main
)

gtest_discover_tests(ustring_test)

add_executable(inline_first_storage_test
    inline_first_storage_tests.cpp
)

target_link_libraries(inline_first_storage_test
    PRIVATE
    GTest::gtest
    GTest::gtest_main
)

add_executable(inline_first_storage_benchmark
    inline_first_storage_benchmark.cpp
)

target_link_libraries(inline_first_storage_benchmark
    PRIVATE
    benchmark::benchmark
)

gtest_discover_tests(inline_first_storage_test)

add_executable(small_vector_test
    small_vector.h
    small_vector_tests.cpp
)

target_link_libraries(small_vector_test
    PRIVATE
    GTest::gtest
    GTest::gtest_main
)

gtest_discover_tests(small_vector_test)

add_executable(small_vector_benchmark
    small_vector.h
    small_vector_benchmark.cpp
)

target_link_libraries(small_vector_benchmark
    PRIVATE
    benchmark::benchmark
)

add_executable(range_formatter_test
    range_formatter.h
    range_formatter_test.cpp
)

target_link_libraries(range_formatter_test
    PRIVATE
    GTest::gtest
    GTest::gtest_main
)

gtest_discover_tests(range_formatter_test)