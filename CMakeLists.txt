cmake_minimum_required(VERSION 3.21)
project(PhantomForce LANGUAGES CXX)

find_package(SFML 2.5 COMPONENTS graphics window system)

add_executable(PhantomForce  src/Main.cpp)

# Add include directory to the project
target_include_directories(PhantomForce PRIVATE include)

# Add source files to the project
target_sources(PhantomForce PRIVATE
          src/Player.cpp
          src/Game.cpp
          src/TileMap.cpp)

# Add assets directory to the project
file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})

target_link_libraries(PhantomForce PRIVATE sfml-graphics)
target_compile_features(PhantomForce PRIVATE cxx_std_17)
if (WIN32 AND BUILD_SHARED_LIBS)
  add_custom_command(TARGET PhantomForce POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:PhantomForce> $<TARGET_FILE_DIR:PhantomForce> COMMAND_EXPAND_LISTS)
endif()

install(TARGETS PhantomForce)
