add_library(helpers INTERFACE)

target_include_directories(helpers INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_compile_features(helpers INTERFACE cxx_std_20)