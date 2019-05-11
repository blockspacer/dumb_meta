# meta_gen tool

function(clang_tooling_exe name srcs extlibs)
add_executable(${name} ${srcs})
target_link_libraries(${name} 
  PRIVATE 
  clangAST
  clangBasic
  clangDriver
  clangFrontend
  clangRewriteFrontend
  clangSerialization
  clangStaticAnalyzerFrontend
  clangTooling
  ${extlibs}
  )

target_include_directories(${name} PUBLIC ${CLANG_INCLUDE_DIRS})

endfunction()

clang_tooling_exe(meta_gen "meta_gen/meta_gen.cpp;meta_gen/path_util_win.cpp;meta_gen/meta_type_handler_rttr.cpp" RTTR::Core_Lib)


# runtime library

add_library(meta_inspection inspection/inspection.cpp inspection/inspection_imgui.cpp inspection/inspector_standard_type.cpp)
target_link_libraries(meta_inspection PUBLIC imgui_impl_dx11 RTTR::Core_Lib)
target_include_directories(meta_inspection PUBLIC ${CMAKE_SOURCE_DIR}/src/)