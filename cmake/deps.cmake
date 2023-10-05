include(FetchContent)

set (FETCHCONTENT_QUIET OFF)

FetchContent_Declare(
    SOEM
    GIT_REPOSITORY https://github.com/OpenEtherCATsociety/SOEM
    GIT_TAG v1.4.0
)
set(BUILD_TESTS OFF)
FetchContent_MakeAvailable(SOEM)
