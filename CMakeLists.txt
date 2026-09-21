cmake_minimum_required(VERSION 3.14)
project(Smart_Pointers CXX)

# Устанавливаем стандарт C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- ОПЦИЯ ДЛЯ ВКЛЮЧЕНИЯ САНИТАЙЗЕРОВ ---
# В CLion можно передать флаг -DENABLE_SANITIZERS=ON в настройках профиля CMake
option(ENABLE_SANITIZERS "Enable Address and Undefined Behavior Sanitizers" OFF)

if(ENABLE_SANITIZERS)
    message(STATUS "Sanitizers ENABLED")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,undefined -g -O1 -fno-omit-frame-pointer")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address,undefined")
endif()

# ==========================================
# 1. СБОРКА ОСНОВНОЙ ПРОГРАММЫ
# ==========================================
# Так как main.cpp лежит в корне, указываем его напрямую
add_executable(main_app main.cpp)

# ==========================================
# 2. НАСТРОЙКА GOOGLE TEST
# ==========================================
enable_testing()
include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
FetchContent_MakeAvailable(googletest)

# ==========================================
# 3. СБОРКА ТЕСТОВ
# ==========================================
add_executable(run_tests
        tests/test_unique.cpp
        tests/test_shared.cpp
        tests/test_array.cpp
)

# Указываем тестам, где искать заголовочные файлы (.hpp).
# CMAKE_SOURCE_DIR указывает на корень проекта.
target_include_directories(run_tests PRIVATE ${CMAKE_SOURCE_DIR})

# Подключаем к исполняемому файлу тестов библиотеку GTest
target_link_libraries(run_tests gtest_main)

# Интеграция с панелью тестов в CLion
include(GoogleTest)
gtest_discover_tests(run_tests)