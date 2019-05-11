add_executable(meta_test meta_test.cpp gui_stub.cpp  meta_store.cpp)

target_link_libraries(meta_test PRIVATE meta_inspection)

set(META_GEN_EXECUTABLE ${CMAKE_BINARY_DIR}/src/meta_gen)

add_custom_command(
TARGET meta_test PRE_BUILD
COMMAND ${META_GEN_EXECUTABLE} dummy.h -src_root="${CMAKE_SOURCE_DIR}/test" -single_meta_storage_path="${CMAKE_SOURCE_DIR}/test/meta_store.cpp" -extra-arg=-DREFLECT -- -x c++ -I"${CMAKE_SOURCE_DIR}/src"
WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)