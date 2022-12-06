cmake_minimum_required(VERSION 3.13) # Обозначим минимальную версию CMake
project(calculator LANGUAGES CXX) # Название и языки проекта

set(CMAKE_CXX_STANDARD 14) # Стандарт C++

# Поддиректории со своими CMakeLists.txt
add_subdirectory(proto)
add_subdirectory(src)
