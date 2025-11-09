include(FetchContent)

FetchContent_Declare(
    oniguruma
    GIT_REPOSITORY https://github.com/kkos/oniguruma.git
    GIT_TAG v6.9.9
)

FetchContent_MakeAvailable(oniguruma)