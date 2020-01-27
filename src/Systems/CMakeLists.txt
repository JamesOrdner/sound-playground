target_sources(SoundPlayground
  PRIVATE
    SystemInterface.cpp
    SystemInterface.h
    SystemObjectInterface.cpp
    SystemObjectInterface.h
    SystemSceneInterface.cpp
    SystemSceneInterface.h
)

add_subdirectory(Audio)
add_subdirectory(Graphics)
add_subdirectory(Input)
add_subdirectory(Physics)
