# Pillow Real Estate API

REST API сервис для управления недвижимостью, реализованный на Yandex Userver.

## Домашнее задание 02: Разработка REST API сервиса

### Выполненные требования

#### 1. Проектирование REST API

Разработаны REST API endpoints для всех операций:

- `POST /api/auth/register` - регистрация пользователя
- `POST /api/auth/login` - вход в систему
- `GET /api/properties` - получение списка объектов (с фильтрацией по городу и цене)
- `GET /api/properties/{id}` - получение объекта по ID
- `POST /api/properties` - создание объекта (требуется аутентификация)
- `PUT /api/properties/{id}` - обновление объекта (только владелец)
- `DELETE /api/properties/{id}` - удаление объекта (только владелец)
- `GET /api/viewings` - получение просмотров пользователя
- `POST /api/viewings` - запись на просмотр
- `PUT /api/viewings/{id}` - обновление записи
- `DELETE /api/viewings/{id}` - отмена записи
- `GET /api/users` - получение информации о текущем пользователе

Использованы правильные HTTP методы (GET, POST, PUT, DELETE). Использованы правильные HTTP статус-коды (200, 201, 400, 401, 403, 404, 409). Структура URL соответствует REST принципам. Разработана структура Request/Response в формате JSON.

#### 2. Реализация REST API сервиса

Реализован на C++20 с использованием фреймворка Yandex Userver. Реализовано более 12 API endpoints. Использовано in-memory хранилище (словари C++). Реализована обработка ошибок с правильными HTTP статус-кодами. Использованы DTO структуры для передачи данных (UserData, Property, Viewing). Архитектура с разделением на сервисы:

- `UserService` - управление пользователями и аутентификация
- `PropertyService` - управление объектами недвижимости
- `ViewingService` - управление просмотрами

#### 3. Реализация аутентификации

Реализована аутентификация на основе JWT-like токенов (формат: jwt_username_timestamp). Защищены endpoints:

- `POST /api/properties` (создание объекта)
- `PUT /api/properties/{id}` (обновление объекта)
- `DELETE /api/properties/{id}` (удаление объекта)
- `POST /api/viewings` (запись на просмотр)
- `PUT /api/viewings/{id}` (обновление записи)
- `DELETE /api/viewings/{id}` (отмена записи)

Реализованы endpoints для регистрации и логина пользователей. Добавлена проверка прав доступа (только владелец может редактировать/удалять свой объект). Токены хранятся в памяти и валидируются при каждом защищенном запросе.

#### 4. Документирование API

Создана OpenAPI 3.0 спецификация (openapi.yaml). Описаны все endpoints с параметрами и схемами запросов/ответов. Ниже добавлены примеры использования.

#### 5. Тестирование

Созданы интеграционные тесты (tests/test_api.sh). Протестированы успешные сценарии: регистрация нового пользователя, логин и получение токена, создание, чтение, обновление, удаление объектов, поиск с фильтрацией по городу и цене. Протестирована обработка ошибок:

- Неверные учетные данные (401)
- Отсутствие аутентификации (401)
- Недостаточные права доступа (403)
- Несуществующие ресурсы (404)
- Конфликт при регистрации (409)
- Неверные входные данные (400)

Все 24 теста проходят успешно.

## Технологии

- C++20
- Yandex Userver (фреймворк для высоконагруженных сервисов)
- JSON для сериализации
- OpenAPI 3.0 для документации
- Docker для контейнеризации

## Структура проекта

```
pillow-api/
├── src/
│   ├── handlers/           # Обработчики HTTP запросов
│   │   ├── auth_handler.cpp/hpp
│   │   ├── property_list_handler.cpp/hpp
│   │   ├── property_item_handler.cpp/hpp
│   │   ├── viewing_handler.cpp/hpp
│   │   └── user_handler.cpp/hpp
│   ├── services/           # Бизнес-логика
│   │   ├── user_service.cpp/hpp
│   │   ├── property_service.cpp/hpp
│   │   └── viewing_service.cpp/hpp
│   ├── utils/              # Вспомогательные функции
│   │   └── auth_utils.cpp/hpp
│   ├── storage.cpp/hpp     # Глобальное хранилище данных
│   └── main.cpp            # Точка входа
├── config/
│   └── config.yaml         # Конфигурация сервиса
├── tests/
│   ├── test_api.sh         # Тесты (24 проверки)
│   ├── run_tests.sh        # Запуск тестов
│   └── README.md           # Документация по тестам
├── openapi.yaml            # OpenAPI спецификация
├── CMakeLists.txt          # Сборка проекта
├── Dockerfile              # Docker образ
├── docker-compose.yaml     # Docker Compose конфигурация
└── README.md               # Данный файл
```

## Запуск сервиса

### Через Docker

```bash
docker-compose up --build
```

Или с помощью Docker:

```bash
docker build -t pillow-api .
docker run -p 8080:8080 -v $(pwd)/config/config.yaml:/etc/pillow/config.yaml:ro pillow-api
```

### Локальный запуск

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
./pillow-api --config ../config/config.yaml
```

## Примеры использования API

### Регистрация пользователя

```bash
curl -X POST http://localhost:8080/api/auth/register \
  -H "Content-Type: application/json" \
  -d '{
    "username": "john_doe",
    "password": "securepass123",
    "email": "john@example.com"
  }'
```

Ответ (201 Created):

```json
{
  "id": "user_1234567890_123456",
  "username": "john_doe",
  "email": "john@example.com",
  "message": "User registered successfully"
}
```

### Вход в систему

```bash
curl -X POST http://localhost:8080/api/auth/login \
  -H "Content-Type: application/json" \
  -d '{
    "username": "john_doe",
    "password": "securepass123"
  }'
```

Ответ (200 OK):

```json
{
  "access_token": "jwt_john_doe_1234567890",
  "token_type": "Bearer",
  "expires_in": 3600,
  "user": {
    "id": "user_1234567890_123456",
    "username": "john_doe"
  }
}
```

### Создание объекта недвижимости

```bash
TOKEN="jwt_john_doe_1234567890"

curl -X POST http://localhost:8080/api/properties \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $TOKEN" \
  -d '{
    "title": "Modern Apartment",
    "description": "Spacious 2-bedroom apartment",
    "price": 250000,
    "city": "Moscow",
    "rooms": 2
  }'
```

Ответ (201 Created):

```json
{
  "id": "1",
  "title": "Modern Apartment",
  "description": "Spacious 2-bedroom apartment",
  "price": 250000,
  "city": "Moscow",
  "rooms": 2,
  "owner_id": "john_doe",
  "created_at": "2024-01-15T10:30:00Z",
  "message": "Property created successfully"
}
```

### Получение списка объектов

```bash
curl "http://localhost:8080/api/properties"
```

### Поиск с фильтрацией

```bash
curl "http://localhost:8080/api/properties?city=Moscow&min_price=200000&max_price=300000"
```

### Получение объекта по ID

```bash
curl "http://localhost:8080/api/properties/1"
```

### Обновление объекта

```bash
curl -X PUT http://localhost:8080/api/properties/1 \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $TOKEN" \
  -d '{
    "price": 230000,
    "title": "Updated Apartment"
  }'
```

Ответ (200 OK):

```json
{
  "id": "1",
  "title": "Updated Apartment",
  "description": "Spacious 2-bedroom apartment",
  "price": 230000,
  "city": "Moscow",
  "rooms": 2,
  "owner_id": "john_doe",
  "created_at": "2024-01-15T10:30:00Z",
  "message": "Property updated successfully"
}
```

### Удаление объекта

```bash
curl -X DELETE http://localhost:8080/api/properties/1 \
  -H "Authorization: Bearer $TOKEN"
```

Ответ (200 OK):

```json
{
  "message": "Property deleted successfully",
  "id": "1"
}
```

### Запись на просмотр

```bash
curl -X POST http://localhost:8080/api/viewings \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $TOKEN" \
  -d '{
    "property_id": "1",
    "scheduled_time": "2024-01-20T15:00:00Z",
    "notes": "I would like to see the apartment"
  }'
```

Ответ (201 Created):

```json
{
  "id": "1",
  "property_id": "1",
  "user_id": "john_doe",
  "scheduled_time": "2024-01-20T15:00:00Z",
  "status": "scheduled",
  "notes": "I would like to see the apartment",
  "created_at": "2024-01-15T10:35:00Z",
  "message": "Viewing scheduled successfully"
}
```

### Получение просмотров пользователя

```bash
curl -X GET http://localhost:8080/api/viewings \
  -H "Authorization: Bearer $TOKEN"
```

### Получение информации о пользователе

```bash
curl -X GET http://localhost:8080/api/users \
  -H "Authorization: Bearer $TOKEN"
```

## Запуск тестов

```bash
cd tests
./run_tests.sh
```

Ожидаемый результат:

```
=========================================
Test Results
=========================================
Passed: 24
Failed: 0
Total: 24
All tests passed!
```