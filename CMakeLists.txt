cmake_minimum_required(VERSION 3.0)
project(bytecode)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -std=c99 -DMARK_AND_COMPACT -Wall")

set(SOURCE src/vm.c src/loader.c src/vm_strings.c)

add_library(vm ${SOURCE})
target_include_directories(vm PUBLIC src)

include_directories(src)

add_library(c_unit test/c_unit.c)
target_include_directories (c_unit PUBLIC test)

add_executable(wrun src/wrun.c)
target_link_libraries(wrun vm)

include(CTest)

add_executable(test_core test/test_core.c)
target_link_libraries(test_core LINK_PUBLIC vm c_unit)
add_test(NAME test_core
        COMMAND    valgrind --error-exitcode=1 --tool=memcheck --leak-check=full ./test_core)

add_executable(test_funcs test/test_funcs.c)
target_link_libraries(test_funcs LINK_PUBLIC vm c_unit)
add_test(NAME test_funcs
        COMMAND    valgrind --error-exitcode=1 --tool=memcheck --leak-check=full ./test_funcs)
