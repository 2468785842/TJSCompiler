include(FetchContent)

FetchContent_Declare(
    oniguruma
    GIT_REPOSITORY https://github.com/kkos/oniguruma.git
    GIT_TAG v6.9.9
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/ext/oniguruma
    BINARY_DIR ${CMAKE_BINARY_DIR}/_deps/oniguruma/build
    SUBBUILD_DIR ${CMAKE_BINARY_DIR}/_deps/oniguruma/subbuild
)

FetchContent_MakeAvailable(oniguruma)