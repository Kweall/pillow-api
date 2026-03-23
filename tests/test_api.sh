#!/bin/bash

# Цвета для вывода
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # Без цвета

BASE_URL="http://localhost:8080"
PASSED=0
FAILED=0
TOKEN=""

# Функция для выполнения теста
test_endpoint() {
    local name="$1"
    local method="$2"
    local endpoint="$3"
    local data="$4"
    local expected_code="$5"
    local auth_required="$6"
    
    echo -n "Testing: $name ... "
    
    local cmd="curl -s -o /tmp/response.txt -w '%{http_code}' -X $method"
    
    if [ -n "$data" ]; then
        cmd="$cmd -H 'Content-Type: application/json' -d '$data'"
    fi
    
    if [ "$auth_required" = "true" ] && [ -n "$TOKEN" ]; then
        cmd="$cmd -H 'Authorization: Bearer $TOKEN'"
    fi
    
    cmd="$cmd $BASE_URL$endpoint"
    
    local http_code=$(eval $cmd)
    local response=$(cat /tmp/response.txt)
    
    if [ "$http_code" = "$expected_code" ]; then
        echo -e "${GREEN}PASSED${NC} (HTTP $http_code)"
        ((PASSED++))
        if [ "$method" = "POST" ] && [ "$endpoint" = "/api/properties" ] && [ "$expected_code" = "201" ]; then
            PROPERTY_ID=$(echo "$response" | jq -r '.id')
        fi
        return 0
    else
        echo -e "${RED}FAILED${NC} (Expected HTTP $expected_code, got $http_code)"
        echo "Response: $response"
        ((FAILED++))
        return 1
    fi
}

echo "========================================="
echo "Pillow API Tests"
echo "========================================="
echo ""

# 1. Тестируем Ping
test_endpoint "Ping endpoint" "GET" "/ping" "" "200" "false"

# 2. Регистрация нового уникального пользователя
USERNAME="testuser_$(date +%s%N)_$$"
echo -n "Testing: Register user - valid ($USERNAME) ... "
RESPONSE_FILE="/tmp/register_response.txt"
HTTP_CODE=$(curl -s -w '%{http_code}' -o "$RESPONSE_FILE" -X POST "$BASE_URL/api/auth/register" \
    -H "Content-Type: application/json" \
    -d "{\"username\":\"$USERNAME\",\"password\":\"test123\",\"email\":\"$USERNAME@example.com\"}")
REGISTER_RESPONSE=$(cat "$RESPONSE_FILE")

if [ "$HTTP_CODE" = "201" ]; then
    echo -e "${GREEN}PASSED${NC} (HTTP $HTTP_CODE)"
    ((PASSED++))
else
    echo -e "${RED}FAILED${NC} (Expected HTTP 201, got $HTTP_CODE)"
    echo "Response: $REGISTER_RESPONSE"
    ((FAILED++))
fi

# 3. Регистрация существующего пользователя
test_endpoint "Register user - duplicate" "POST" "/api/auth/register" \
    "{\"username\":\"$USERNAME\",\"password\":\"test123\",\"email\":\"$USERNAME@example.com\"}" "409" "false"

# 4. Регистрация с неверными данными
test_endpoint "Register user - short username" "POST" "/api/auth/register" \
    '{"username":"a","password":"test123","email":"test@example.com"}' "400" "false"

# 5. Регистрация с неверным email
test_endpoint "Register user - invalid email" "POST" "/api/auth/register" \
    '{"username":"testuser2","password":"test123","email":"invalid"}' "400" "false"

# 6. Логин
echo -n "Testing: Login ... "

RESPONSE_FILE="/tmp/login_response.txt"
HTTP_CODE=$(curl -s -w '%{http_code}' -o "$RESPONSE_FILE" -X POST "$BASE_URL/api/auth/login" \
    -H "Content-Type: application/json" \
    -d "{\"username\":\"$USERNAME\",\"password\":\"test123\"}")
LOGIN_RESPONSE=$(cat "$RESPONSE_FILE")

if [ "$HTTP_CODE" = "200" ]; then
    echo -e "${GREEN}PASSED${NC} (HTTP $HTTP_CODE)"
    ((PASSED++))
    TOKEN=$(echo "$LOGIN_RESPONSE" | jq -r '.access_token')
    echo "Token obtained: ${TOKEN:0:40}..."
else
    echo -e "${RED}FAILED${NC} (Expected HTTP 200, got $HTTP_CODE)"
    ((FAILED++))
fi

# 7. Логин с неверным паролем
test_endpoint "Login - wrong password" "POST" "/api/auth/login" \
    "{\"username\":\"$USERNAME\",\"password\":\"wrongpass\"}" "401" "false"

# 8. Создание объекта недвижимости
test_endpoint "Create property - valid" "POST" "/api/properties" \
    '{"title":"Test Apartment","price":250000,"city":"Moscow","rooms":2}' "201" "true"

# Сохраняем ID созданного объекта
if [ -f /tmp/response.txt ]; then
    PROPERTY_ID=$(cat /tmp/response.txt | jq -r '.id')
    echo "Property ID from response: $PROPERTY_ID"
fi

# Если не получили ID из ответа, получаем последний из списка
if [ -z "$PROPERTY_ID" ] || [ "$PROPERTY_ID" = "null" ]; then
    PROPERTY_ID=$(curl -s "$BASE_URL/api/properties" | jq -r '.[-1].id')
    echo "Property ID from list: $PROPERTY_ID"
fi

# 9. Создание объекта без токена
test_endpoint "Create property - no auth" "POST" "/api/properties" \
    '{"title":"Test Apartment","price":250000,"city":"Moscow","rooms":2}' "401" "false"

# 10. Создание объекта с неверными данными
test_endpoint "Create property - invalid price" "POST" "/api/properties" \
    '{"title":"Test","price":-100,"city":"Moscow","rooms":2}' "400" "true"

# 11. Получение списка объектов
test_endpoint "Get properties list" "GET" "/api/properties" "" "200" "false"

# 12. Получение конкретного объекта
if [ -n "$PROPERTY_ID" ] && [ "$PROPERTY_ID" != "null" ]; then
    test_endpoint "Get property by ID" "GET" "/api/properties/$PROPERTY_ID" "" "200" "false"
else
    echo -e "${YELLOW}Skipping get by ID - no property ID available${NC}"
fi

# 13. Получение несуществующего объекта
test_endpoint "Get non-existent property" "GET" "/api/properties/999999" "" "404" "false"

# 14. Поиск с фильтрацией
test_endpoint "Search properties - by city" "GET" "/api/properties?city=Moscow" "" "200" "false"

# 15. Поиск с фильтрацией по цене
test_endpoint "Search properties - by price range" "GET" "/api/properties?min_price=200000&max_price=300000" "" "200" "false"

# 16. Обновление объекта
if [ -n "$PROPERTY_ID" ] && [ "$PROPERTY_ID" != "null" ]; then
    test_endpoint "Update property" "PUT" "/api/properties/$PROPERTY_ID" \
        '{"price":300000,"title":"Updated Apartment"}' "200" "true"
else
    echo -e "${YELLOW}Skipping update - no property ID available${NC}"
fi

# 17. Обновление чужого объекта (создадим второго пользователя)
SECOND_USER="testuser2_$TIMESTAMP"
curl -s -X POST "$BASE_URL/api/auth/register" \
    -H "Content-Type: application/json" \
    -d "{\"username\":\"$SECOND_USER\",\"password\":\"test123\",\"email\":\"$SECOND_USER@example.com\"}" > /dev/null 2>&1

SECOND_TOKEN=$(curl -s -X POST "$BASE_URL/api/auth/login" \
    -H "Content-Type: application/json" \
    -d "{\"username\":\"$SECOND_USER\",\"password\":\"test123\"}" | jq -r '.access_token')

# Сохраняем текущий токен и используем второй
OLD_TOKEN=$TOKEN
TOKEN=$SECOND_TOKEN
if [ -n "$PROPERTY_ID" ] && [ "$PROPERTY_ID" != "null" ]; then
    test_endpoint "Update property - forbidden" "PUT" "/api/properties/$PROPERTY_ID" \
        '{"price":100000}' "403" "true"
else
    echo -e "${YELLOW}Skipping forbidden update - no property ID available${NC}"
fi
TOKEN=$OLD_TOKEN

# 18. Удаление объекта
if [ -n "$PROPERTY_ID" ] && [ "$PROPERTY_ID" != "null" ]; then
    test_endpoint "Delete property" "DELETE" "/api/properties/$PROPERTY_ID" "" "200" "true"
else
    echo -e "${YELLOW}Skipping delete - no property ID available${NC}"
fi

# 19. Удаление несуществующего объекта
test_endpoint "Delete non-existent property" "DELETE" "/api/properties/999999" "" "404" "true"

# 20. Удаление без токена
test_endpoint "Delete property - no auth" "DELETE" "/api/properties/1" "" "401" "false"

# 21. Создание второго объекта для проверки списка
test_endpoint "Create second property" "POST" "/api/properties" \
    '{"title":"Another Apartment","price":180000,"city":"Moscow","rooms":1}' "201" "true"

# 22. Получение списка после создания
test_endpoint "Get properties after creation" "GET" "/api/properties" "" "200" "false"

# 23. Проверка создания объекта с максимальными комнатами
test_endpoint "Create property - max rooms" "POST" "/api/properties" \
    '{"title":"Large Apartment","price":500000,"city":"Moscow","rooms":10}' "201" "true"

# 24. Проверка создания объекта с комнатами > 10
test_endpoint "Create property - rooms > 10" "POST" "/api/properties" \
    '{"title":"Too Large","price":500000,"city":"Moscow","rooms":11}' "400" "true"

echo ""
echo "========================================="
echo "Test Results"
echo "========================================="
echo -e "${GREEN}Passed: $PASSED${NC}"
echo -e "${RED}Failed: $FAILED${NC}"
echo "Total: $((PASSED + FAILED))"

if [ $FAILED -eq 0 ]; then
    echo -e "${GREEN}All tests passed! 🎉${NC}"
    exit 0
else
    echo -e "${RED}Some tests failed! ❌${NC}"
    exit 1
fi