project(Tests C CXX ASM)

include(FetchContent)

# Use GoogleTest from source
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Dependancy: Embedded Template Library
FetchContent_Declare(
  etl
  GIT_REPOSITORY https://github.com/ETLCPP/etl
  GIT_TAG        20.43.4
)
FetchContent_MakeAvailable(etl)


# Testing executable
add_executable(tests 
    test_efft_fixed_unknown.cpp
)
include_directories(tests INTERFACE "${FETCHCONTENT_BASE_DIR}/googletest-src/googletest/include") 
include_directories(tests INTERFACE "${PROJECT_SOURCE_DIR}/include") 
include_directories(etl INTERFACE "${FETCHCONTENT_BASE_DIR}/etl-src/include") # header only library

# Linking to GoogleTest
target_link_libraries(
  tests
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(tests)

