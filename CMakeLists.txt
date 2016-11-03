cmake_minimum_required(VERSION 2.8.11)
# Name of the project
set(PROJECT_NAME Packman)
project(${PROJECT_NAME})
#Bring the headers into the project (local ones)
include_directories(include $ENV{HOME}/NGL/include)

#the file(GLOB...) allows for wildcard additions of our src dir
set(SOURCES main.c  map.h )

# see what platform we are on and set platform defines
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	add_definitions(-DDARWIN)
	find_library(MACGL OpenGL)
	set ( PROJECT_LINK_LIBS ${MACGL})

elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	add_definitions(-DLINUX)
	set ( PROJECT_LINK_LIBS -lGL)

endif()

# As there are no proper cmake packages for SDL2 yet we use the sdl2-config script instead
# this is a good example of how to run a program / script in cmake
# exec_program exe arguments and where we place the output
exec_program(sdl2-config ARGS --cflags OUTPUT_VARIABLE SDL2INCLUDE)
# once we have the output for the include path we set this to the CXX
SET( CMAKE_C_FLAGS ${SDL2INCLUDE})
message("sdl2-config --cflags gives " =${SDL2INCLUDE})

exec_program(sdl2-config ARGS --libs OUTPUT_VARIABLE SDL2_LIBRARIES)
message("sdl2-config --libs gives " =${SDL2_LIBRARIES})


# add exe and link libs this must be after the other defines
add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} ${PROJECT_LINK_LIBS} ${SDL2_LIBRARIES} -lSDL2_image  )

