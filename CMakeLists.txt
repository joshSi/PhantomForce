cmake_minimum_required(VERSION 3.16)
project(PhantomForce LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(
  SOURCES
  src/main.cpp
  src/Game.cpp
  src/Player.cpp
  src/TileMap.cpp
  src/Object.cpp
  src/menu/Button.cpp
  # src/menu/PauseOverlay.cpp
)
set(
  HEADERS
  include/utils/utils.h
  include/utils/platform_utils.h
  include/Game.h
  include/Player.h
  include/TileMap.h
  include/Object.h
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
  add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${SOURCES} ${HEADERS} ${RESOURCES})
  target_link_libraries(${PROJECT_NAME} PRIVATE SFML::Graphics "-framework CoreFoundation")

   # Set variables for Info.plist
  set(MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME})  # Short name
  set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.jsi.${PROJECT_NAME}")
  set(MACOSX_BUNDLE_BUNDLE_VERSION "1.0")
  set(MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0")

  # Generate the Info.plist file
  configure_file(${CMAKE_SOURCE_DIR}/Info.plist.in ${CMAKE_BINARY_DIR}/Info.plist @ONLY)
  set_target_properties(${PROJECT_NAME} PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_BINARY_DIR}/Info.plist
    INSTALL_RPATH "@executable_path/../Frameworks"
    BUILD_WITH_INSTALL_RPATH TRUE
  )
elseif(WIN32)
  file(COPY ${RESOURCE_PATH} DESTINATION ${CMAKE_BINARY_DIR})
  add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
  target_link_libraries(${PROJECT_NAME} PRIVATE SFML::Graphics)
else()
  file(COPY ${RESOURCE_PATH} DESTINATION ${CMAKE_BINARY_DIR})
  add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${RESOURCES})
  target_link_libraries(${PROJECT_NAME} PRIVATE SFML::Graphics)
endif()
target_include_directories(${PROJECT_NAME} PRIVATE include include/utils include/menu include/assets)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
if (WIN32 AND BUILD_SHARED_LIBS)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}> $<TARGET_FILE_DIR:${PROJECT_NAME}> COMMAND_EXPAND_LISTS)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${RESOURCE_PATH}/ $<TARGET_FILE_DIR:${PROJECT_NAME}>/${RESOURCE_PATH}/ COMMAND_EXPAND_LISTS)
endif()

if (APPLE)
  install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin)
elseif(WIN32)
  install(TARGETS ${PROJECT_NAME})
else()
  install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin)
endif()

install(DIRECTORY ${RESOURCE_PATH}/ DESTINATION ${RESOURCE_PATH})
if (APPLE)
  install(DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.app/Contents/Resources/ DESTINATION Resources)
else()
  install(DIRECTORY ${RESOURCE_PATH}/ DESTINATION ${CMAKE_INSTALL_PREFIX}/${RESOURCE_PATH})
endif()
