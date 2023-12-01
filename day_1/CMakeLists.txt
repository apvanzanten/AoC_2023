cmake_minimum_required(VERSION 3.13)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(AoC_Template C)

set(CMAKE_C_STANDARD 11)

# --- fetch c facilities ---
include(FetchContent)
FetchContent_Declare(
  c_facilities
  GIT_REPOSITORY https://github.com/apvanzanten/c_facilities/
  GIT_TAG        origin/main
  )
FetchContent_MakeAvailable(c_facilities)
set(CFAC_INC_DIR ${c_facilities_SOURCE_DIR}/inc)
include_directories(${CFAC_INC_DIR})

# --- build ---
set(WARNINGS 
    -Wall; 
    -Wextra; 
    -pedantic; 
    -Werror;)

set(SANITIZERS 
    -fsanitize=address; 
    -fsanitize=leak; 
    -fsanitize=undefined;
    -fno-omit-frame-pointer;
    )
  
set(FLAGS
    -Og;
    -g;
    -DDEBUG;
    )

add_compile_options(${WARNINGS} ${SANITIZERS} ${FLAGS})
add_link_options(${SANITIZERS})

link_libraries(log)

add_library(lib lib.c)
target_link_libraries(lib span darray m)

add_executable(main main.c)
target_link_libraries(main lib)

enable_testing()

function(AddTest TEST_NAME TEST_SOURCE #[[test dependencies...]])
    add_executable(${TEST_NAME} ${TEST_SOURCE})

    target_link_libraries(${TEST_NAME} test_utils ${ARGN})

    add_test(${TEST_NAME} ${TEST_NAME})
endfunction()

AddTest(lib_test lib.test.c lib)
