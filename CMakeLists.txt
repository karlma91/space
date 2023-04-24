cmake_minimum_required(VERSION 3.7)

project(space)

if(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
endif()

# to manually select install locations of libraries and executables
#   -D LIB_INSTALL_DIR mylib
#   -D BIN_INSTALL_DIR newbin
set(LIB_INSTALL_DIR lib CACHE STRING "Install location of libraries")
set(BIN_INSTALL_DIR bin CACHE STRING "Install location of executables")


if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
  option(FORCE_CLANG_BLOCKS "Force enable Clang blocks" YES)
endif()

if(NOT MSVC)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99") # always use gnu99
  if(FORCE_CLANG_BLOCKS)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fblocks")
  endif()
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -ffast-math") # extend release-profile with fast-math
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall") # extend debug-profile with -Wall
endif()


set(SPACE_VERSION_MAJOR 1)
set(SPACE_VERSION_MINOR 0)
set(SPACE_VERSION_PATCH 0)
set(SPACE_VERSION "${SPACE_VERSION_MAJOR}.${SPACE_VERSION_MINOR}.${SPACE_VERSION_PATCH}")
message("Configuring SPACE2D version ${SPACE_VERSION}")

find_package(GLEW REQUIRED)
find_package(SDL2 REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)
message("SDL2_IMAGE: ${SDL2_IMAGE_LIBRARIES}")
if(NOT SDL2_IMAGE_FOUND)
  message(FATAL_ERROR "SDL2_IMAGE not found")
endif ()

pkg_check_modules(SDL2_MIXER REQUIRED SDL2_mixer)
message("SDL2_MIXER: ${SDL2_MIXER_LIBRARIES}")
if(NOT SDL2_MIXER_FOUND)
  message(FATAL_ERROR "SDL2_MIXER not found")
endif ()

pkg_check_modules(CJSON REQUIRED libcjson)
message("CJSON: ${CJSON_LIBRARIES}")
if(NOT CJSON_FOUND)
  message(FATAL_ERROR "CJSON not found")
endif ()

find_library(CHIPMUNK_LIBRARY NAMES libchipmunk.a HINTS usr/local/lib)
find_path(CHIPMUNK_INCLUDE_DIRS chipmunk/chipmunk.h)
message("CHIPMUNK_INCLUDE_DIRS: ${CHIPMUNK_INCLUDE_DIRS}")
if(NOT CHIPMUNK_LIBRARY)
  message(FATAL_ERROR "CHIPMUNK_LIBRARY library not found")
endif ()

pkg_check_modules(ZZIP REQUIRED zziplib)
message("ZZIP_LIBRARIES: ${ZZIP_LIBRARIES}")
if(NOT ZZIP_FOUND)
  message(FATAL_ERROR "zziplib not found")
endif ()

set(space_include_dirs
	${GLEW_INCLUDE_DIR}
  ${SDL2_INCLUDE_DIRS}
  ${SDL2_IMAGE_INCLUDE_DIRS}
  ${SDL2_MIXER_INCLUDE_DIRS}
  ${CHIPMUNK_INCLUDE_DIRS}/chipmunk
  ${ZZIP_INCLUDE_DIRS}
  ${CJSON_INCLUDE_DIRS}
  "src/engine/include"
)

set(space_libraries
	${GLEW_LIBRARIES}
  ${SDL2_LIBRARIES}
  ${SDL2_IMAGE_LIBRARIES}
  ${SDL2_MIXER_LIBRARIES}
  ${ZZIP_LIBRARIES}
  ${CJSON_LIBRARIES}
  ${CHIPMUNK_LIBRARY}
)

file(GLOB_RECURSE space_source_files RELATIVE "${CMAKE_SOURCE_DIR}" "src/*.c")
list(APPEND space_source_files "tools/encrypt/we_crypt.c")
message("Source: ${space_source_files}")
message("space_include_dirs: ${space_include_dirs}")
list(APPEND space_libraries X11 Xi GL mxml inih)
list(APPEND space_libraries m pthread)
message("space_libraries: ${space_libraries}")

include_directories(${space_include_dirs})
link_directories(${space_library_dirs})
add_executable(space ${space_source_files})
target_link_libraries(space ${space_libraries})
