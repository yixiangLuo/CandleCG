cmake_minimum_required(VERSION 3.1)
project(soil)

find_package (OpenGL REQUIRED)
include_directories (${OPENGL_INCLUDE_DIR})
set(LIBRARIES ${OPENGL_LIBRARIES})

ADD_LIBRARY( soil STATIC
  src/image_helper.c
  src/stb_image_aug.c
  src/image_DXT.c
  src/SOIL.c
  )
target_include_directories(soil PUBLIC inc)
target_link_libraries (soil ${LIBRARIES})
