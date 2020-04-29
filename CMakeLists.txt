cmake_minimum_required (VERSION 2.6)
project (pathfinding)

# add the binary tree to the search path for include files
# so that we will find header files
include_directories (
	"./"
	"${PROJECT_BINARY_DIR}"
	)

# Specify directories in which the linker will look for libraries.
link_directories("../../glut")

# add the executable
add_executable (pathfinding pathfinding.cpp)
target_link_libraries (pathfinding  ${EXTRA_LIBS})
