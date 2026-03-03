cmake_minimum_required(VERSION 3.20)
project(AlgorithmicArena VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

option(USE_QUADTREE_COLLISION "Use Quadtree collision detection" ON)
option(USE_ASTAR_PATHFINDING "Use A* pathfinding" ON)

if(USE_QUADTREE_COLLISION)
  add_compile_definitions(USE_QUADTREE_COLLISION)
else()
  add_compile_definitions(USE_BRUTE_FORCE_COLLISION)
endif()

if(USE_ASTAR_PATHFINDING)
  add_compile_definitions(USE_ASTAR_PATHFINDING)
else()
  add_compile_definitions(USE_DIJKSTRAS_PATHFINDING)
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(SFML REQUIRED sfml-system sfml-window sfml-graphics)

# file(GLOB_RECURSE SOURCES
#     "src/*.cpp"
#     "src/World/*.cpp"
#     "src/Entities/*.cpp"
#     "src/Algorithms/Collision/*.cpp"
#     "src/Algorithms/Pathfinding/*.cpp"
#     "src/Core/*.cpp"
#     "src/UI/*.cpp"
# )
add_executable(${PROJECT_NAME}
    src/main.cpp
    src/Game.cpp
    src/Entities/Minion.cpp
    src/Entities/PlayerCommander.cpp
    src/World/TileMap.cpp
    src/World/MapLoader.cpp
)

# add_executable(${PROJECT_NAME} ${SOURCES})

find_package(spdlog REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE ${SFML_LIBRARIES} spdlog::spdlog)
target_include_directories(${PROJECT_NAME} PRIVATE ${SFML_INCLUDE_DIRS})
target_compile_options(${PROJECT_NAME} PRIVATE ${SFML_CFLAGS_OTHER})

target_include_directories(${PROJECT_NAME} PRIVATE src)

