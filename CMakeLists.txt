cmake_minimum_required(VERSION 3.16)
project(PhantomForce LANGUAGES CXX)

find_package(SFML 2.6 COMPONENTS graphics window system)

set(SOURCES src/Main.cpp src/Game.cpp src/Player.cpp src/TileMap.cpp)
set(HEADERS include/utils.h include/platform_utils.h include/Game.h include/Player.h include/TileMap.h)
set(RESOURCE_PATH assets)
set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install)
file(GLOB_RECURSE RESOURCES "${RESOURCE_PATH}/*")

if (APPLE)
  set_source_files_properties(${RESOURCES} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
  add_executable(PhantomForce MACOSX_BUNDLE ${SOURCES} ${HEADERS} ${RESOURCES})
  target_link_libraries(PhantomForce PRIVATE sfml-graphics "-framework CoreFoundation")
elseif(WIN32)
  if(NOT SFML_FOUND)
    message("SFML not found, downloading...")
    include(FetchContent)
    FetchContent_Declare(SFML
      GIT_REPOSITORY https://github.com/SFML/SFML.git
      GIT_TAG 2.6.x
    )
    FetchContent_MakeAvailable(SFML)
  endif()
  file(COPY ${RESOURCE_PATH} DESTINATION ${CMAKE_BINARY_DIR})
  add_executable(PhantomForce ${SOURCES} ${HEADERS})
  target_link_libraries(PhantomForce PRIVATE sfml-graphics)
else()
  # TODO: Linux resources
  add_executable(PhantomForce ${SOURCES} ${HEADERS} ${RESOURCES})
  target_link_libraries(PhantomForce PRIVATE sfml-graphics)
endif()
target_include_directories(PhantomForce PRIVATE include)

target_compile_features(PhantomForce PRIVATE cxx_std_17)
if (WIN32 AND BUILD_SHARED_LIBS)
  add_custom_command(TARGET PhantomForce POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:PhantomForce> $<TARGET_FILE_DIR:PhantomForce> COMMAND_EXPAND_LISTS)
endif()

if (APPLE)
  install(TARGETS PhantomForce
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin)
elseif(WIN32)
  install(TARGETS PhantomForce)
else()
  install(TARGETS PhantomForce
    RUNTIME DESTINATION bin)
endif()
