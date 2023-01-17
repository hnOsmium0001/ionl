cmake_minimum_required(VERSION 3.13)
project(Ionl LANGUAGES CXX)

option(Ionl_DEBUG_FEATURES "Master flag for enabling debugging features" OFF)

find_package(fmt CONFIG REQUIRED)
find_package(date CONFIG REQUIRED)
find_package(robin_hood CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(Freetype REQUIRED)
find_package(SQLite3 REQUIRED)

file(GLOB_RECURSE IonlApp_SRC_FILES
    source/*.h
    source/*.hpp
    source/*.c
    source/*.cpp
)
add_executable(IonlApp ${IonlApp_SRC_FILES})

target_compile_features(IonlApp PRIVATE cxx_std_20)

target_include_directories(IonlApp
PRIVATE
    sources/
)

target_link_libraries(IonlApp
PRIVATE
    fmt::fmt
    date::date date::date-tz
    robin_hood::robin_hood
    glad::glad
    glfw
    freetype
    SQLite::SQLite3
)

target_compile_definitions(IonlApp
PRIVATE
    IONL_DEBUG_FEATURES=$<BOOL:${Ionl_DEBUG_FEATURES}>
)
