include(ExternalProject)

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
target_link_libraries(SoundPlayground PRIVATE ${DEPENDENCIES_DIR}/lib/fftw3f.lib)

# ---------- PortAudio

target_include_directories(SoundPlayground PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/portaudio/include)
target_link_libraries(SoundPlayground PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/portaudio/lib/portaudio_x64.lib)

# ---------- SDL2

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
target_include_directories(SoundPlayground PRIVATE ${DEPENDENCIES_DIR}/include/SDL2)
target_link_libraries(SoundPlayground PRIVATE ${DEPENDENCIES_DIR}/lib/SDL2.lib ${DEPENDENCIES_DIR}/lib/SDL2main.lib)

# ---------- TinyGLTF

target_include_directories(SoundPlayground PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tinygltf)

# ---------- GL3W

target_include_directories(SoundPlayground PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/gl3w/include)
target_sources(SoundPlayground PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/gl3w/src/gl3w.c)