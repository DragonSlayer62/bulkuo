cmake_minimum_required(VERSION 3.18)
project(action VERSION 1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

add_library(action STATIC
    actions.cpp
    actutility.cpp
    create.cpp
    exist.cpp
    extract.cpp
    merge.cpp
    names.cpp
    uophash.cpp
    actions.hpp
    actutility.hpp
    create.hpp
    exist.hpp
    extract.hpp
    merge.hpp
    names.hpp
    uophash.hpp

)
target_include_directories(action
		PUBLIC
			${PROJECT_SOURCE_DIR}/../utility
)

if (WIN32)
	target_compile_options( action PRIVATE
		$<$<CONFIG:Release>:/O2>
	)
	
else()

	if (APPLE)
		target_compile_options( action PRIVATE
			$<$<CONFIG:Release>:-Os>
		)
	else()
		target_compile_options(action PRIVATE
			$<$<CONFIG:Release>:-O2>
		)
	endif(APPLE)
endif(WIN32)




