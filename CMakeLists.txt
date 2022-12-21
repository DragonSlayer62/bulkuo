cmake_minimum_required(VERSION 3.18)
project(bulkuo VERSION 0.1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

# *************************************************************************
# The basic executable we are making
# *************************************************************************

add_executable(bulkuo
    source/main.cpp
    $<$<STREQUAL:${CMAKE_SYSTEM_NAME},Windows>:${PROJECT_SOURCE_DIR}/assets/Resource.rc>
)

# *************************************************************************
# Platform specific settings
# *************************************************************************

if (WIN32)
set (CMAKE_RC_STANDARD_INCLUDE_DIRECTORIES ${PROJECT_SOURCE_DIR}/assets/)
	
target_compile_definitions(bulkuo PRIVATE
		_CONSOLE
		$<$<CONFIG:Release>:NDEBUG>
		$<$<CONFIG:Debug>:_DEBUG>
	)
	
	target_compile_options( bulkuo PRIVATE
		$<$<CONFIG:Release>:/O2>
	)
else()

	target_compile_definitions( bulkuo PRIVATE
		$<$<CONFIG:Debug>:_DEBUG>
	)
	
	target_compile_options( bulkuo PRIVATE
	)
	if (APPLE)
		target_compile_options( bulkuo PRIVATE
			$<$<CONFIG:Release>:-Os>
		)
	else()
		target_compile_options(bulkuo PRIVATE
			$<$<CONFIG:Release>:-O2>
			-Wno-attributes
		)
	endif(APPLE)
endif(WIN32)



# *************************************************************************
# the directories we need on the include path
# *************************************************************************

target_include_directories(bulkuo
	PUBLIC
		${PROJECT_SOURCE_DIR}/source
		${PROJECT_SOURCE_DIR}/source/utility
)

# *************************************************************************
# The libraries we need
# *************************************************************************
target_link_libraries(bulkuo PRIVATE
	bulk
	artwork
	uodata
	action
	zlib
	$<$<PLATFORM_ID:Windows>:ws2_32>
	$<$<PLATFORM_ID:Windows>:Kernel32>
	$<$<PLATFORM_ID:Linux>:pthread>
)

# *************************************************************************
# The items we need built first
# *************************************************************************
add_subdirectory(${PROJECT_SOURCE_DIR}/source/zlib subproject/zlib)
add_subdirectory(${PROJECT_SOURCE_DIR}/source/artwork subproject/artwork)
add_subdirectory(${PROJECT_SOURCE_DIR}/source/uodata subproject/uodata)
add_subdirectory(${PROJECT_SOURCE_DIR}/source/action subproject/action)
add_subdirectory(${PROJECT_SOURCE_DIR}/source subproject/bulk)
