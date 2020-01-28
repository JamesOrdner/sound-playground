cmake_minimum_required(VERSION 3.16)

project(SoundPlayground)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(SoundPlayground "")

add_subdirectory(src)

get_property("TARGET_SOURCE_FILES" TARGET SoundPlayground PROPERTY SOURCES)
source_group(TREE "${CMAKE_CURRENT_LIST_DIR}" FILES ${TARGET_SOURCE_FILES})

# external dependencies

set(DEPENDENCIES_DIR ${PROJECT_SOURCE_DIR}/dependencies)
add_subdirectory(lib)

# copy dlls

add_custom_command(TARGET SoundPlayground POST_BUILD 
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
  "${DEPENDENCIES_DIR}/bin/fftw3f.dll"              
  $<TARGET_FILE_DIR:SoundPlayground>
)

add_custom_command(TARGET SoundPlayground POST_BUILD 
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
  "${CMAKE_CURRENT_SOURCE_DIR}/lib/portaudio/lib/portaudio_x64.dll"              
  $<TARGET_FILE_DIR:SoundPlayground>
)

add_custom_command(TARGET SoundPlayground POST_BUILD 
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
  "${DEPENDENCIES_DIR}/bin/SDL2.dll"              
  $<TARGET_FILE_DIR:SoundPlayground>
)

# copy resources

add_custom_command(TARGET SoundPlayground POST_BUILD 
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  "${CMAKE_CURRENT_SOURCE_DIR}/res"              
  ${CMAKE_BINARY_DIR}/res
)
