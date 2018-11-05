cmake_minimum_required(VERSION 3.1)
project(final_project)

################################################################################

# Directory to external libraries used in the project
set(THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/)

# Project sources
add_executable(${PROJECT_NAME}
	src/main.cpp
	# src/helpers.cpp
	# src/helpers.h
)

# Use C++11 version of the standard
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON)

# Place the output binary at the root of the build folder
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

# Include Eigen for linear algebra
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC "${THIRD_PARTY_DIR}/eigen")

# Include GLFW3 for windows management
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL " " FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL " " FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL " " FORCE)
set(GLFW_BUILD_INSTALL OFF CACHE BOOL " " FORCE)
add_subdirectory("${THIRD_PARTY_DIR}/glfw" glfw)
target_link_libraries(${PROJECT_NAME} glfw)

# Include glew
# add_subdirectory("${THIRD_PARTY_DIR}/glew" glew)
# target_link_libraries(${PROJECT_NAME} libglew_static)

# Include glad
add_subdirectory("${THIRD_PARTY_DIR}/glad" glad)
target_link_libraries(${PROJECT_NAME} glad)

# Include glm
add_subdirectory("${THIRD_PARTY_DIR}/glm" glm)
target_link_libraries(${PROJECT_NAME} glm)

# Include SOIL
add_subdirectory("${THIRD_PARTY_DIR}/soil" soil)
target_link_libraries(${PROJECT_NAME} soil)

# Include assimp
add_subdirectory("${THIRD_PARTY_DIR}/assimp" assimp)
target_link_libraries(${PROJECT_NAME} assimp)

# Folder where data files are stored (meshes & stuff)
set(DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/data/")
target_compile_definitions(${PROJECT_NAME} PUBLIC -DDATA_DIR=\"${DATA_DIR}\")

set(SHADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/")
target_compile_definitions(${PROJECT_NAME} PUBLIC -DSHADER_DIR=\"${SHADER_DIR}\")

set(MODEL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/model/")
target_compile_definitions(${PROJECT_NAME} PUBLIC -DMODEL_DIR=\"${MODEL_DIR}\")
