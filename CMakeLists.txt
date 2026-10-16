cmake_minimum_required(VERSION 3.16.0)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
set(EXTRA_COMPONENT_DIRS src)
idf_build_set_property(DEPENDENCIES_LOCK dependencies.lock.${IDF_TARGET})
project(esp32-current-clamp)

idf_build_get_property(build_dir BUILD_DIR)
idf_build_get_property(elf_name EXECUTABLE_NAME GENERATOR_EXPRESSION)
set(app_bin "${build_dir}/${elf_name}.bin")
set(app_elf "${build_dir}/${elf_name}.elf")
