# CMakeLists.txt for Snowball stemmer library
cmake_minimum_required(VERSION 3.18)

# Use the snowball compiler from the snowball submodule
# The snowball_compiler target is defined in external/snowball/CMakeLists.txt

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
    
    # Generate C file using snowball compiler from snowball submodule
    add_custom_command(
        OUTPUT ${OUTPUT_DIR}/stem_UTF_8_${ALGORITHM_NAME}.c
        COMMAND $<TARGET_FILE:snowball_compiler>
                ${ALGORITHM_FILE} -o ${OUTPUT_DIR}/stem_UTF_8_${ALGORITHM_NAME} -eprefix ${ALGORITHM_NAME}_UTF_8_ -r ${CMAKE_CURRENT_SOURCE_DIR}/runtime -u
        DEPENDS snowball_compiler ${ALGORITHM_FILE}
    )
    list(APPEND GENERATED_SOURCES ${OUTPUT_DIR}/stem_UTF_8_${ALGORITHM_NAME}.c)
endforeach()

# Generate modules.h from modules.txt
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/modules.h
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/libstemmer/mkmodules.pl
            modules.h
            src_c
            ${CMAKE_CURRENT_SOURCE_DIR}/libstemmer/modules.txt
            mkinc.mak
            utf8
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
