cmake_minimum_required(VERSION 3.20)
project(ShootEmUp
    LANGUAGES C
)

#setting c version to 99
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

#variables
set(EXECUTABLE_NAME game)

#required libraries
find_package(SDL2 REQUIRED)

include_directories(${PROJECT_NAME} ${SDL2_INCLUDE_DIRS})

#setting up the executable
add_executable(${EXECUTABLE_NAME})
target_sources(${EXECUTABLE_NAME}
    PRIVATE
        main.c

        #internal
        i_main.h
        i_main.c

        #render
        r_main.h
        r_main.c

        #auido
        mix_main.h
        mix_main.c

        #GUI
        ui_main.h
        ui_main.c

        #gameplay
        p_main.h
        p_main.c
)

target_link_libraries(${EXECUTABLE_NAME}
    ${SDL2_LIBRARIES}
    SDL2_image
    SDL2_mixer
)
