cmake_minimum_required( VERSION 3.10 )

project( coding )

set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib" )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib" )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin" )

# exporting of llvm compiler_commands.json enabled
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

# setup compiler
add_compile_options( -g )
set( hwsvc_CXX_WARNING_FLAGS -Wall -Wextra -Wconversion -Wshadow -Wnon-virtual-dtor -Wold-style-cast -Wcast-align -Wcast-qual -Wunused -Woverloaded-virtual -Wno-noexcept-type -Wpedantic )
if ( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" )
  set( hwsvc_CXX_WARNING_FLAGS ${hwsvc_CXX_WARNING_FLAGS} -Weverything -Wno-c++98-compat-pedantic  -Wno-c++98-compat -Wno-documentation -Wno-reserved-id-macro -Wno-switch-enum -Wno-weak-vtables -Wno-missing-prototypes -Wno-padded -Wno-missing-noreturn -Wno-exit-time-destructors -Wno-documentation-unknown-command -Wno-unused-template -Wno-undef -Wno-format-nonliteral -Wno-global-constructors)
else( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" )
  set( hwsvc_CXX_WARNING_FLAGS ${hwsvc_CXX_WARNING_FLAGS} -Wnoexcept )
endif( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" )
  set( hwsvc_CXX_WARNING_FLAGS ${hwsvc_CXX_WARNING_FLAGS} -std=c++17 )

add_library( coding
  STATIC
    src/bit_buffer.cpp
    src/compr_stats.cpp
    src/data_block.cpp
    src/fib_coding.cpp
    src/freq_table.cpp
    src/num_freq_table.cpp
    src/num_freq_table_adapt.cpp
    src/num_freq_table_alias.cpp
    src/rans.cpp

    src/common.h
    src/bit_buffer.h
    src/compr_stats.h
    src/data_block.h
    src/fib_coding.h
    src/freq_table.h
    src/num_freq_table.h
    src/num_freq_table_adapt.h
    src/num_freq_table_alias.h
    src/rans.h )

target_include_directories( coding
  PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src" )

target_compile_options( coding
  PUBLIC
    ${hwsvc_CXX_WARNING_FLAGS} )

#target_link_libraries( ${HWSVC_MODULE_TARGET_NAME}
#  PUBLIC
#    ${HWSVC_MODULE_DEPENDENCIES} )

add_executable( base
    tests/base_test.cpp )

target_compile_options( base
  PUBLIC
    ${hwsvc_CXX_WARNING_FLAGS} )

target_link_libraries( base
  PUBLIC
    coding )

# ---

add_executable( rans
    tests/rans_basic_test.cpp )

target_compile_options( rans
  PUBLIC
    ${hwsvc_CXX_WARNING_FLAGS} )

target_link_libraries( rans
  PUBLIC
    coding )

# ---

add_executable( alias
    tests/alias_test.cpp )

target_compile_options( alias
  PUBLIC
    ${hwsvc_CXX_WARNING_FLAGS} )

target_link_libraries( alias
  PUBLIC
    coding )

# ---

add_executable( fib
    tests/fib_test.cpp )

target_compile_options( fib
  PUBLIC
    ${hwsvc_CXX_WARNING_FLAGS} )

target_link_libraries( fib
  PUBLIC
    coding )

# ---

add_executable( adapt
    tests/adapt_test.cpp )

target_compile_options( adapt
  PUBLIC
    ${hwsvc_CXX_WARNING_FLAGS} )

target_link_libraries( adapt
  PUBLIC
    coding )
