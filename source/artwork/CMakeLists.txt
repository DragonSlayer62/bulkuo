cmake_minimum_required(VERSION 3.18)
project(artwork VERSION 1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

add_library(artwork STATIC
    animation.cpp
    art.cpp
    gump.cpp
    hue.cpp
    texture.cpp
    light.cpp
    animation.hpp
    art.hpp
    gump.hpp
    hue.hpp
    texture.hpp
    light.hpp
)
target_include_directories(artwork
		PUBLIC
			${PROJECT_SOURCE_DIR}/../utility
)

if (WIN32)
	target_compile_options( artwork PRIVATE
		$<$<CONFIG:Release>:/O2>
	)
	
else()

	if (APPLE)
		target_compile_options( artwork PRIVATE
			$<$<CONFIG:Release>:-Os>
		)
	else()
		target_compile_options(artwork PRIVATE
			$<$<CONFIG:Release>:-O2>
		)
	endif(APPLE)
endif(WIN32)




