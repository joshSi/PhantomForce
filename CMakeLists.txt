cmake_minimum_required(VERSION 3.21)
project(PhantomForce LANGUAGES CXX)

find_package(SFML 2.5 COMPONENTS graphics window system)

set(SOURCES src/Main.cpp src/Game.cpp src/Player.cpp src/TileMap.cpp)
set(HEADERS include/utils.h include/platform_utils.h include/Game.h include/Player.h include/TileMap.h)
set(RESOURCE_PATH assets)
set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install)
file(GLOB_RECURSE RESOURCES "${RESOURCE_PATH}/*")
set_source_files_properties(${RESOURCES} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
if (APPLE)
  add_executable(PhantomForce MACOSX_BUNDLE ${SOURCES} ${HEADERS} ${RESOURCES})
elseif(WIN32)
  add_executable(PhantomForce WIN32 ${SOURCES} ${HEADERS} ${RESOURCES})
else()
  add_executable(PhantomForce ${SOURCES} ${HEADERS} ${RESOURCES})
endif()
target_include_directories(PhantomForce PRIVATE include)

target_link_libraries(PhantomForce PRIVATE sfml-graphics "-framework CoreFoundation")
target_compile_features(PhantomForce PRIVATE cxx_std_17)
if (WIN32 AND BUILD_SHARED_LIBS)
  add_custom_command(TARGET PhantomForce POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:PhantomForce> $<TARGET_FILE_DIR:PhantomForce> COMMAND_EXPAND_LISTS)
endif()

install(TARGETS PhantomForce
  BUNDLE DESTINATION .
  RUNTIME DESTINATION bin)
