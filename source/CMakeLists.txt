cmake_minimum_required(VERSION 3.18)
project(bulk VERSION 1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

add_library(bulk STATIC
    argument.cpp
    types.cpp
    idlist.cpp
    catlabel.cpp
    argument.hpp
    types.hpp
    idlist.hpp
    catlabel.hpp
)
target_include_directories(bulk
		PUBLIC
			${PROJECT_SOURCE_DIR}/utility
)

if (WIN32)
	target_compile_options( bulk PRIVATE
		$<$<CONFIG:Release>:/O2>
	)
	
else()

	if (APPLE)
		target_compile_options( bulk PRIVATE
			$<$<CONFIG:Release>:-Os>
		)
	else()
		target_compile_options(bulk PRIVATE
			$<$<CONFIG:Release>:-O2>
		)
	endif(APPLE)
endif(WIN32)




