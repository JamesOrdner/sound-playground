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

add_subdirectory(lib)

include(ExternalProject)
set(DEPENDENCIES_DIR ${PROJECT_SOURCE_DIR}/dependencies)
set(DEPENDENCY_CMAKE_ARGS
  -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
  -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
  -DCMAKE_BUILD_TYPE=Release
)

# ---------- FFTW

set(FFTW_CACHE_ARGS
  -DENABLE_FLOAT:BOOL=TRUE
  -DBUILD_TESTS:BOOL=FALSE
)
ExternalProject_Add(FFTW
  URL http://www.fftw.org/fftw-3.3.8.tar.gz
  PREFIX ${DEPENDENCIES_DIR}
  CMAKE_ARGS ${DEPENDENCY_CMAKE_ARGS}
  CMAKE_CACHE_ARGS ${FFTW_CACHE_ARGS}
  INSTALL_COMMAND ${CMAKE_COMMAND} --build . --target install --config Release
)
target_include_directories(SoundPlayground PRIVATE ${DEPENDENCIES_DIR}/include)
if (WIN32)
  target_link_libraries(SoundPlayground PRIVATE ${DEPENDENCIES_DIR}/lib/fftw3f.lib)
elseif(APPLE)
  target_link_libraries(SoundPlayground PRIVATE ${DEPENDENCIES_DIR}/lib/libfftw3f.dylib)
endif()

# ---------- PortAudio

if (APPLE)
  ExternalProject_Add(PortAudio
    URL http://www.portaudio.com/archives/pa_stable_v190600_20161030.tgz
    PREFIX ${DEPENDENCIES_DIR}
    CMAKE_ARGS ${DEPENDENCY_CMAKE_ARGS}
    INSTALL_COMMAND ${CMAKE_COMMAND} --build . --target install --config Release
  )
  target_link_libraries(SoundPlayground PRIVATE ${DEPENDENCIES_DIR}/lib/libportaudio.dylib)
endif()

# ---------- SDL2

if (WIN32) # use precompiled binaries
  ExternalProject_Add(SDL2
    URL https://www.libsdl.org/release/SDL2-devel-2.0.10-VC.zip
    PREFIX ${DEPENDENCIES_DIR}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND
      COMMAND ${CMAKE_COMMAND} -E copy_directory ${DEPENDENCIES_DIR}/src/SDL2/lib/x64 ${DEPENDENCIES_DIR}/lib
      COMMAND ${CMAKE_COMMAND} -E copy_directory ${DEPENDENCIES_DIR}/src/SDL2/include ${DEPENDENCIES_DIR}/include/SDL2
  )
  target_link_libraries(SoundPlayground PRIVATE ${DEPENDENCIES_DIR}/lib/SDL2.lib ${DEPENDENCIES_DIR}/lib/SDL2main.lib)
elseif(APPLE) # build from source
  set(SDL2_CACHE_ARGS
    -DSDL_STATIC:BOOL=FALSE
    -DSDL_SHARED:BOOL=TRUE
    -DSDL_Audio:BOOL=FALSE
    -DSDL_File:BOOL=FALSE
    -DSDL_Filesystem:BOOL=FALSE
    -DSDL_Threads:BOOL=FALSE
    -DSDL_Joystick:BOOL=FALSE
    -DSDL_Haptic:BOOL=FALSE
    -DSDL_Atomic:BOOL=FALSE
    -DSDL_Render:BOOL=FALSE
    -DSDL_Sensor:BOOL=FALSE
    -DDIRECTX:BOOL=FALSE
  )
  ExternalProject_Add(SDL2
    URL http://www.libsdl.org/release/SDL2-2.0.10.tar.gz
    PREFIX ${DEPENDENCIES_DIR}
    CMAKE_ARGS ${DEPENDENCY_CMAKE_ARGS}
    CMAKE_CACHE_ARGS ${SDL2_CACHE_ARGS}
    INSTALL_COMMAND ${CMAKE_COMMAND} --build . --target install --config Release
  )
  target_link_libraries(SoundPlayground PRIVATE ${DEPENDENCIES_DIR}/lib/libSDL2.dylib ${DEPENDENCIES_DIR}/lib/libSDL2main.a)
endif()
target_include_directories(SoundPlayground PRIVATE ${DEPENDENCIES_DIR}/include/SDL2)

if (WIN32) # copy dlls
  add_custom_command(TARGET SoundPlayground POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${DEPENDENCIES_DIR}/bin/fftw3f.dll"              
    $<TARGET_FILE_DIR:SoundPlayground>
  )
  add_custom_command(TARGET SoundPlayground POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${DEPENDENCIES_DIR}/lib/SDL2.dll"              
    $<TARGET_FILE_DIR:SoundPlayground>
  )
  add_custom_command(TARGET SoundPlayground POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/portaudio/lib/portaudio_x64.dll"              
    $<TARGET_FILE_DIR:SoundPlayground>
  )
endif()

# copy resources

if(WIN32)
  add_custom_command(TARGET SoundPlayground POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/res
    ${CMAKE_BINARY_DIR}/res
  )
else()
  add_custom_command(TARGET SoundPlayground POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/res
    $<TARGET_FILE_DIR:SoundPlayground>/res
  )
endif()
