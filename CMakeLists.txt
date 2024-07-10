cmake_minimum_required(VERSION 3.25)
project(PongGame)

set(CMAKE_CXX_STANDARD 20)

add_executable(PongGame main.cpp
        Game/Game.cpp
        Game/Game.h
        Game/Ball.cpp
        Game/Ball.h
        Game/Paddle.cpp
        Game/Paddle.h
        Game/PlayerScore.cpp
        Game/PlayerScore.h
        Game/Vec2.cpp
        Game/Vec2.h
        Game/Indicator.cpp
        Game/Indicator.h
        Game/constants.h
        FixedQueue.h
)

find_package( OpenCV REQUIRED )
include_directories(${OpenCV_INCLUDE_DIRS})
target_link_libraries(PongGame ${OpenCV_LIBS})

include_directories("/usr/include/SDL2")
target_link_libraries(PongGame "/usr/lib/aarch64-linux-gnu/libSDL2.so")
target_link_libraries(PongGame "/usr/lib/aarch64-linux-gnu/libSDL2_ttf.so")
