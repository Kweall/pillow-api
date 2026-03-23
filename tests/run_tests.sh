#!/bin/bash

echo "========================================="
echo "Running Pillow API Tests"
echo "========================================="
echo ""

echo "Checking if service is running..."
if curl -s http://localhost:8080/ping > /dev/null 2>&1; then
    echo "Service is running. Starting tests..."
    echo ""
    
    # Запускаем тесты
    ./test_api.sh
    
    EXIT_CODE=$?
    
    echo ""
    if [ $EXIT_CODE -eq 0 ]; then
        echo "All tests completed successfully!"
    else
        echo "Some tests failed. Check the output above."
    fi
    
    exit $EXIT_CODE
else
    echo "ERROR: Service is not running on http://localhost:8080"
    echo "Please start the service first:"
    echo "  cd /workspace/pillow-api"
    echo "  ./build/pillow-api --config config/config.yaml"
    exit 1
fi
