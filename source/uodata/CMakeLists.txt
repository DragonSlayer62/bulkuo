cmake_minimum_required(VERSION 3.18)
project(uodata VERSION 1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

add_library(uodata STATIC
    flag.cpp
    hash.cpp
    hueaccess.cpp
    idx.cpp
    multicollection.cpp
    tileinfo.cpp
    uop.cpp
    uowave.cpp
    flag.hpp
    hash.hpp
    hueaccess.hpp
    idx.hpp
    multicollection.hpp
    tileinfo.hpp
    uop.hpp
    uowave.hpp
)
target_include_directories(uodata
		PUBLIC
			${PROJECT_SOURCE_DIR}/../utility
			${PROJECT_SOURCE_DIR}/../zlib
			
)
	target_link_libraries(uodata
        zlib
    )

if (WIN32)
	target_compile_options( uodata PRIVATE
		$<$<CONFIG:Release>:/O2>
	)
    target_include_directories(bulkuo
		PUBLIC
			${PROJECT_SOURCE_DIR}/../zlib
	)


else()

	if (APPLE)
		target_compile_options( uodata PRIVATE
			$<$<CONFIG:Release>:-Os>
		)
	else()
		target_compile_options(uodata PRIVATE
			$<$<CONFIG:Release>:-O2>
		)
        target_link_libraries(uodata
            zlib
        )
	endif(APPLE)
endif(WIN32)



