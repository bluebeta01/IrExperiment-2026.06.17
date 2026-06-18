cmake_minimum_required(VERSION 3.21)
project(ircompiler)
file(GLOB_RECURSE sources src/*.h src/*.c)
set(target_name ircompiler)
set(CMAKE_C_STANDARD 17)
add_executable(${target_name} ${sources})
set_property(TARGET ${target_name} PROPERTY C_STANDARD 23)
set_target_properties(${target_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY bin)

target_link_libraries(${target_name}
	
)