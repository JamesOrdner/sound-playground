target_include_directories(SoundPlayground PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/fftw/include)
target_link_libraries(SoundPlayground PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/fftw/lib/libfftw3f-3.lib)

target_include_directories(SoundPlayground PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/portaudio/include)
target_link_libraries(SoundPlayground PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/portaudio/lib/portaudio_x64.lib)

target_include_directories(SoundPlayground PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/SDL2/include)
target_link_libraries(SoundPlayground
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/SDL2/lib/SDL2.lib
    ${CMAKE_CURRENT_SOURCE_DIR}/SDL2/lib/SDL2main.lib
)

target_include_directories(SoundPlayground PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tinygltf)

target_include_directories(SoundPlayground PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/gl3w/include)
target_sources(SoundPlayground PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/gl3w/src/gl3w.c)