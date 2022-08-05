cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(Sigil)

# The project uses local static initialization. Since Zephyr doesn't support the
# C++ ABI for thread-safe initialization of local statics and the constructors don't
# appear to require thread safety, we turn it off in the C++ compiler.
set(NO_THREADSAFE_STATICS $<TARGET_PROPERTY:compiler-cpp,no_threadsafe_statics>)
zephyr_compile_options($<$<COMPILE_LANGUAGE:CXX>:${NO_THREADSAFE_STATICS}>)

# TODO: List all the sources
target_sources(app
    PRIVATE
        src/accelerometer_handler.cpp
        src/assert.cpp
        src/gesture_predictor.cpp
        src/magic_wand_model_data.cpp
        src/main.cpp
        src/main_functions.cpp
        src/output_handler.cpp
        src/accelerometer_handler.hpp
        src/constants.hpp
        src/gesture_predictor.hpp
        src/magic_wand_model_data.hpp
        src/main_functions.hpp
        src/output_handler.hpp
)
