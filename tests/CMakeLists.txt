# Module unit tests (not copied into projects). Built standalone linked against the engine core (mealkit_core):
#   cmake -S tests -B build/tests -G Ninja -DMEALKIT_ROOT=<engine folder> -DMEALKIT_HEADLESS_ONLY=ON
#   cmake --build build/tests && build\tests\bin\procgen_tests
# Module code is read with #include "procgen/..." like in a project (forwarding headers are made in the build folder).
cmake_minimum_required(VERSION 3.25)
project(procgen_tests LANGUAGES CXX)
file(TO_CMAKE_PATH "${MEALKIT_ROOT}" MEALKIT_ROOT)
include(${MEALKIT_ROOT}/cmake/MealKit.cmake)

get_filename_component(module_dir "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)
file(GLOB_RECURSE module_headers CONFIGURE_DEPENDS "${module_dir}/sim/*.h" "${module_dir}/view/*.h")
foreach(h IN LISTS module_headers)
    file(RELATIVE_PATH rel "${module_dir}" "${h}")
    file(CONFIGURE OUTPUT "${CMAKE_BINARY_DIR}/inc/procgen/${rel}" CONTENT "#include \"${h}\"\n" @ONLY)
endforeach()

add_executable(procgen_tests ProcgenTests.cpp)
target_link_libraries(procgen_tests PRIVATE mealkit_core)
target_include_directories(procgen_tests PRIVATE "${CMAKE_BINARY_DIR}/inc")
