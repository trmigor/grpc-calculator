# Подключаем необходимые пакеты
find_package(Threads)

# Обозначаем группу исходных файлов
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

# Добавляем файлы
add_executable(client client.cpp)
add_executable(server server.cpp)

# Подключаем нашу библиотеку
target_link_libraries(client
    PRIVATE
        proto
)
target_link_libraries(server
    PRIVATE
        proto
)
