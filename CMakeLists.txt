# CMakeLists.txt for Snowball stemmer library
cmake_minimum_required(VERSION 3.18)

# First, we need to build the snowball compiler
set(SNOWBALL_COMPILER_SOURCES
    compiler/space.c
    compiler/tokeniser.c
    compiler/analyser.c
    compiler/generator.c
    compiler/generator_ada.c
    compiler/generator_csharp.c
    compiler/generator_go.c
    compiler/generator_java.c
    compiler/generator_js.c
    compiler/generator_pascal.c
    compiler/generator_php.c
    compiler/generator_python.c
    compiler/generator_rust.c
    compiler/driver.c
)

# Build the snowball compiler
add_executable(snowball_compiler ${SNOWBALL_COMPILER_SOURCES})
target_include_directories(snowball_compiler PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/runtime
)

# Generate all stemmer source files first
set(ALGORITHMS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/algorithms)
set(OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/src_c)

# Create output directory
file(MAKE_DIRECTORY ${OUTPUT_DIR})

# Find all .sbl files
file(GLOB ALGORITHM_FILES "${ALGORITHMS_DIR}/*.sbl")

# Generate C files from .sbl files
set(GENERATED_SOURCES)
foreach(ALGORITHM_FILE ${ALGORITHM_FILES})
    get_filename_component(ALGORITHM_NAME ${ALGORITHM_FILE} NAME_WE)
    
    # Generate C file
    add_custom_command(
        OUTPUT ${OUTPUT_DIR}/${ALGORITHM_NAME}.c
        COMMAND ${CMAKE_CURRENT_BINARY_DIR}/snowball_compiler
                ${ALGORITHM_FILE} -o ${OUTPUT_DIR}/${ALGORITHM_NAME}
        DEPENDS snowball_compiler ${ALGORITHM_FILE}
    )
    list(APPEND GENERATED_SOURCES ${OUTPUT_DIR}/${ALGORITHM_NAME}.c)
endforeach()

# Generate modules.h from modules.txt
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/modules.h
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/libstemmer/mkmodules.pl
            ${CMAKE_CURRENT_BINARY_DIR}/modules.h
            ${OUTPUT_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/libstemmer/modules.txt
            ${CMAKE_CURRENT_BINARY_DIR}/mkinc.mak
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/libstemmer/mkmodules.pl
            ${CMAKE_CURRENT_SOURCE_DIR}/libstemmer/modules.txt
            ${GENERATED_SOURCES}
)
add_custom_target(stemmer_modules DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/modules.h)

# Generate libstemmer.c from template
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/libstemmer.c
    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_SOURCE_DIR}/libstemmer/libstemmer_c.in
    ${CMAKE_CURRENT_BINARY_DIR}/libstemmer.c
    COMMAND sed -i 's/@MODULES_H@/modules.h/' ${CMAKE_CURRENT_BINARY_DIR}/libstemmer.c
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/libstemmer/libstemmer_c.in ${CMAKE_CURRENT_BINARY_DIR}/modules.h
)

# Runtime sources
set(RUNTIME_SOURCES
    runtime/api.c
    runtime/utilities.c
)

# Algorithm sources (all generated C implementations)
# Use the generated sources from the build directory

# Create the libstemmer library with runtime + algorithms + dispatcher
add_library(stemmer STATIC 
    ${CMAKE_CURRENT_BINARY_DIR}/libstemmer.c
    ${RUNTIME_SOURCES}
    ${GENERATED_SOURCES}
)
add_dependencies(stemmer snowball_compiler stemmer_modules)

# Ensure the library can be linked into shared libs
set_target_properties(stemmer PROPERTIES POSITION_INDEPENDENT_CODE ON C_STANDARD 99 C_STANDARD_REQUIRED ON)

# Set include directories
target_include_directories(stemmer PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/runtime
    ${CMAKE_CURRENT_BINARY_DIR}
    ${OUTPUT_DIR}
)

# Set compile definitions
target_compile_definitions(stemmer PUBLIC 
    COMPILE_TIME_DETERMINISTIC=1
)

# Dependencies
add_dependencies(stemmer snowball_compiler)
