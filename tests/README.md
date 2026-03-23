# Tests for Pillow API

## Running Tests

```bash
# Запуск всех тестов
./run_tests.sh

# Или напрямую
./test_api.sh
```

## Test Coverage

- User registration (valid, duplicate, invalid)
- User login (valid, invalid)
- Property CRUD operations
- Authentication (with/without token)
- Authorization (owner only)
- Search with filters (city, price range)
- Error handling (400, 401, 403, 404, 409)
- Input validation (price, rooms, username, email)

## Requirements

- Service running on http://localhost:8080
- `curl` and `jq` installed

## Expected Output

All 24 tests should pass with green "PASSED" messages.