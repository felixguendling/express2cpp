cmake_minimum_required(VERSION 3.17)
project(express2cpp)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

if (MSVC)
    # PDB debug information is not supported by buildcache.
    string(REPLACE "/Zi" "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
    string(REPLACE "/Zi" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    string(REPLACE "/Zi" "" CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO}")
    string(REPLACE "/Zi" "" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
endif ()

include(cmake/buildcache.cmake)
include(cmake/pkg.cmake)

option(EXPRESS2CPP_LINT "Run clang-tidy with the compiler." OFF)
if (EXPRESS2CPP_LINT)
    include(cmake/clang-tidy.cmake)
endif ()

configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/test_dir.h.in
        ${CMAKE_BINARY_DIR}/generated/test_dir.h
)
add_library(test-dir INTERFACE)
target_include_directories(test-dir INTERFACE ${CMAKE_BINARY_DIR}/generated)

file(GLOB express-files express/src/*.cc)
add_library(express ${express-files})
target_include_directories(express PUBLIC express/include)
target_link_libraries(express boost-filesystem boost cista utl)
target_compile_features(express PUBLIC cxx_std_17)

file(GLOB express-gen-files express/exe/express_gen.cc)
add_executable(express-gen ${express-gen-files})
target_link_libraries(express-gen boost cista express)
target_compile_features(express-gen PUBLIC cxx_std_17)

file(GLOB express-test-files express/test/*.cc)
add_executable(express-test ${express-test-files})
target_include_directories(express-test PUBLIC include)
target_link_libraries(express-test doctest test-dir express)
target_compile_features(express-test PUBLIC cxx_std_17)

file(GLOB step-files step/src/*.cc)
add_library(step ${step-files})
target_include_directories(step PUBLIC step/include)
target_link_libraries(step boost cista utl)
target_compile_features(step PUBLIC cxx_std_17)

function(express2cpp express-file lib)
    add_custom_command(
            COMMAND express-gen ${CMAKE_CURRENT_SOURCE_DIR}/${express-file} ${CMAKE_CURRENT_BINARY_DIR}/${lib}
            DEPENDS express-gen
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${lib}/${lib}.cc
    )
    add_library(${lib} ${CMAKE_CURRENT_BINARY_DIR}/${lib}/${lib}.cc)
    if (MSVC)
        target_compile_options(${lib} PRIVATE /bigobj)
    endif ()
    target_link_libraries(${lib} step utl cista boost)
    target_include_directories(${lib} PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/${lib}/include ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/include)
    target_compile_features(${lib} PUBLIC cxx_std_17)
    set_target_properties(${lib} PROPERTIES CXX_CLANG_TIDY "")
endfunction()

express2cpp(express/test/ifc23.txt ifc23)
file(GLOB step-test-files step/test/*.cc)
add_executable(step-test ${step-test-files})
target_include_directories(step-test PUBLIC include)
target_link_libraries(step-test doctest test-dir step ifc23)
target_compile_features(step-test PUBLIC cxx_std_17)
