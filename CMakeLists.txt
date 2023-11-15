cmake_minimum_required(VERSION 3.12)
project(TestServer)

# Установить стандарт C++ на C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Определяем суфикс для версии Debug
set(CMAKE_DEBUG_POSTFIX _g)

include_directories(include)

# Добавить клиентскую часть
add_subdirectory(client)

# Добавить серверную часть
add_subdirectory(server)
