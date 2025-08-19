cmake_minimum_required(VERSION 3.16)
project(PhantomForce LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(
  SOURCES
  src/main.cpp
  src/Game.cpp
  src/Player.cpp
  src/TileMap.cpp
  src/Object.cpp
  src/menu/Button.cpp
  src/menu/PauseOverlay.cpp
)
set(
  HEADERS
  include/utils/utils.h
  include/utils/platform_utils.h
  include/Game.h
  include/Player.h
  include/TileMap.h
  include/Object.h
  include/menu/PauseOverlay.h
  include/menu/Button.h
)
set(RESOURCE_PATH assets)
set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install)
file(GLOB_RECURSE RESOURCES "${RESOURCE_PATH}/*")

find_package(SFML 3.0.1 COMPONENTS Graphics System QUIET)
if(NOT SFML_FOUND)
  include(FetchContent)
  FetchContent_Declare(SFML
      GIT_REPOSITORY https://github.com/SFML/SFML.git
      GIT_TAG 3.0.1
      GIT_SHALLOW ON
      EXCLUDE_FROM_ALL
      SYSTEM)
  FetchContent_MakeAvailable(SFML)
endif()

if (APPLE)
  set_source_files_properties(${RESOURCES} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
  add_executable(PhantomForce MACOSX_BUNDLE ${SOURCES} ${HEADERS} ${RESOURCES})
  target_link_libraries(PhantomForce PRIVATE SFML::Graphics "-framework CoreFoundation")
elseif(WIN32)
  file(COPY ${RESOURCE_PATH} DESTINATION ${CMAKE_BINARY_DIR})
  add_executable(PhantomForce ${SOURCES} ${HEADERS})
  target_link_libraries(PhantomForce PRIVATE SFML::Graphics)
else()
  file(COPY ${RESOURCE_PATH} DESTINATION ${CMAKE_BINARY_DIR})
  add_executable(PhantomForce ${SOURCES} ${HEADERS} ${RESOURCES})
  target_link_libraries(PhantomForce PRIVATE SFML::Graphics)
endif()
target_include_directories(PhantomForce PRIVATE include include/utils include/menu)

target_compile_features(PhantomForce PRIVATE cxx_std_17)
if (WIN32 AND BUILD_SHARED_LIBS)
  add_custom_command(TARGET PhantomForce POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_RUNTIME_DLLS:PhantomForce> $<TARGET_FILE_DIR:PhantomForce> COMMAND_EXPAND_LISTS)
  add_custom_command(TARGET PhantomForce POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${RESOURCE_PATH}/ $<TARGET_FILE_DIR:PhantomForce>/${RESOURCE_PATH}/ COMMAND_EXPAND_LISTS)
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
