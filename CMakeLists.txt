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
find_package(mimalloc CONFIG REQUIRED)
#find_package(Hyperscan REQUIRED)

add_compile_options(/utf-8 /fsanitize=address /JMC)

add_compile_definitions(-DMIMALLOC_VERBOSE=1)

link_libraries(ICU::uc ICU::i18n ICU::data ICU::in)# TBB::tbb)
link_libraries(TBB::tbb)

link_libraries($<IF:$<TARGET_EXISTS:mimalloc-static>,mimalloc-static,mimalloc>)

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
)

add_executable(ustring_test
    ustring_construction_test.cpp
    ustring_iterator_test.cpp
    ustring_visit_test.cpp
    ustring_modification_test.cpp
    ustring_search_test.cpp
    ustring_transform_test.cpp
    ustring_format_test.cpp
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

gtest_discover_tests(inline_first_storage_test)

add_executable(inline_first_storage_benchmark
    inline_first_storage_benchmark.cpp
)

target_link_libraries(inline_first_storage_benchmark
    PRIVATE
    benchmark::benchmark
)

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

#add_executable(range_formatter_test
#    range_formatter.h
#    range_formatter_test.cpp
#)
#
#target_link_libraries(range_formatter_test
#    PRIVATE
#    GTest::gtest
#    GTest::gtest_main
#)
#
#gtest_discover_tests(range_formatter_test)

add_executable(compiler_features_test
    compiler_features.h
    compiler_features_test.cpp
)

target_link_libraries(compiler_features_test
    PRIVATE
    GTest::gtest
    GTest::gtest_main
)
  
gtest_discover_tests(compiler_features_test)
  
add_executable(basic_concepts_test
    basic_concepts.h
    basic_concepts_test.cpp
)

target_link_libraries(basic_concepts_test
    PRIVATE
    GTest::gtest
    GTest::gtest_main
)
  
gtest_discover_tests(basic_concepts_test)
  
add_executable(signal_test
    basic_signal.h
    basic_signal.cpp
    basic_signal_test.cpp
)

target_link_libraries(signal_test
    PRIVATE
    GTest::gtest
    GTest::gtest_main
)
  
gtest_discover_tests(signal_test)
  
add_executable(unique_id_test
    unique_id.h
    unique_id_test.cpp
)

target_link_libraries(unique_id_test
    PRIVATE
    GTest::gtest
    GTest::gtest_main
)
  
gtest_discover_tests(unique_id_test)

add_executable(long_integer_test
    unique_id.h
    uuid.h
    uuid_test.cpp
    long_integer.h
    long_integer_test.cpp
)

target_link_libraries(long_integer_test
    PRIVATE
    GTest::gtest
    GTest::gtest_main
)
  
gtest_discover_tests(long_integer_test)
  