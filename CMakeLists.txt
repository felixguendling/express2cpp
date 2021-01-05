cmake_minimum_required(VERSION 3.17)
project(express2cpp)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

include(cmake/buildcache.cmake)
include(cmake/pkg.cmake)

configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/test/test_dir.h.in
        ${CMAKE_BINARY_DIR}/generated/test_dir.h
)
add_library(ifc-generated INTERFACE)
target_include_directories(ifc-generated INTERFACE ${CMAKE_BINARY_DIR}/generated)

file(GLOB express-lib-files src/*.cc)
add_library(express-lib ${express-lib-files})
target_include_directories(express-lib PUBLIC include)
target_link_libraries(express-lib boost-filesystem boost cista utl)
target_compile_features(express-lib PUBLIC cxx_std_17)

file(GLOB express-gen-files exe/express_gen.cc)
add_executable(express-gen ${express-gen-files})
target_link_libraries(express-gen cista express-lib)
target_compile_features(express-gen PUBLIC cxx_std_17)

function(express2cpp express-file lib)
    add_custom_command(
            COMMAND express-gen ${CMAKE_CURRENT_SOURCE_DIR}/${express-file} ${CMAKE_CURRENT_BINARY_DIR}/${lib}
            DEPENDS express-gen
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${lib}/${lib}.cc
    )
    add_library(${lib} ${CMAKE_CURRENT_BINARY_DIR}/${lib}/${lib}.cc)
    target_link_libraries(${lib} utl cista)
    target_include_directories(${lib} PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/${lib}/include ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_compile_features(${lib} PUBLIC cxx_std_17)
endfunction()

express2cpp(test/ifc23.txt ifc23)
file(GLOB express-test-files test/*.cc)
add_executable(express-test ${express-test-files})
target_include_directories(express-test PUBLIC include)
target_link_libraries(express-test doctest ifc-generated express-lib ifc23)
target_compile_features(express-test PUBLIC cxx_std_17)