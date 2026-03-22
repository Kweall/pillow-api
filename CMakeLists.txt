cmake_minimum_required(VERSION 3.14)
project(pillow-api CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Отключаем все ненужные компоненты
set(USERVER_FEATURE_POSTGRESQL OFF)
set(USERVER_FEATURE_MONGOOSE OFF)
set(USERVER_FEATURE_REDIS OFF)
set(USERVER_FEATURE_CLICKHOUSE OFF)
set(USERVER_FEATURE_GRPC OFF)
set(USERVER_FEATURE_KAFKA OFF)
set(USERVER_FEATURE_RABBITMQ OFF)
set(USERVER_FEATURE_MYSQL OFF)
set(USERVER_FEATURE_SQLITE OFF)
set(USERVER_FEATURE_ODBC OFF)
set(USERVER_FEATURE_PQ OFF)
set(USERVER_FEATURE_MONGO OFF)

# Находим userver
find_package(userver REQUIRED COMPONENTS core)

# Список исходных файлов
set(SOURCES
    src/main.cpp
    src/handlers/auth_handler.cpp
    src/handlers/property_handler.cpp
    src/handlers/viewing_handler.cpp
    src/handlers/user_handler.cpp
)

# Добавляем исполняемый файл
add_executable(pillow-api ${SOURCES})

# Подключаем userver
target_link_libraries(pillow-api userver::core)

# Добавляем директории для include
target_include_directories(pillow-api PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Флаги компиляции
target_compile_options(pillow-api PRIVATE -Wall -Wextra -Wno-unused-parameter)