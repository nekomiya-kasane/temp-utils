cmake_minimum_required (VERSION 3.28)

project ("ustring")

include(D:/scoop/apps/vcpkg/2024.12.16/scripts/buildsystems/vcpkg.cmake)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#add_subdirectory(thirdparty/icu-cmake)
set(ICU_INCLUDE_DIR )
find_package(ICU REQUIRED COMPONENTS uc in i18n data)
find_package(TBB REQUIRED)
find_package(GTest REQUIRED)
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
)

add_executable(ustring_test
    ustring_construction_test.cpp
    ustring_iterator_test.cpp
    ustring_visit_test.cpp
    ustring_modification_test.cpp
    ustring_search_test.cpp
    ustring_transform_test.cpp
)

target_link_libraries(ustring_test
    PRIVATE
    ustring
    GTest::gtest
    GTest::gtest_main
)

gtest_discover_tests(ustring_test)

add_executable(small_vector_test
    small_vector_test.cpp
)

target_link_libraries(small_vector_test
    PRIVATE
    GTest::gtest
    GTest::gtest_main
)

gtest_discover_tests(small_vector_test)