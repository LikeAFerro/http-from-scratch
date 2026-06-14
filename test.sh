#!/usr/bin/env bash
# Improved test script for the http-from-scratch project.

HTTP=./http-from-scratch
LOG_FILE="test.log"
PASS=0
FAIL=0
[ -n "$1" ] && [ "$1" = "valgrind" ] && HTTP="valgrind --leak-check=full --error-exitcode=1 $HTTP"

# Clear previous log
echo "" > "$LOG_FILE"

# Helper to log and check just the exit code
check_response() {
  $HTTP &
  server_PID=$!

  sleep 1 # Give the server a moment to start

  desc=$1
  expected=$2
  path=$3
  shift 3

  RESPONSE=$(curl -s -o /dev/null -w "%{http_code}" \
    "$@" \
    "http://127.0.0.1:8080/$path")

  kill $server_PID
  wait $server_PID 2> /dev/null

  if [ "$RESPONSE" -eq "$expected" ]; then
    echo "[PASS] $desc (HTTP $RESPONSE)" >> "$LOG_FILE"
    echo -e "  [\033[32mPASS\033[0m] $desc"
    ((++PASS))
  else
    echo "[FAIL] $desc (Expected HTTP $expected, got $RESPONSE)" >> "$LOG_FILE"
    echo -e "  [\033[31mFAIL\033[0m] $desc (Expected HTTP $expected, got $RESPONSE. Check test.log)"
    ((++FAIL))
  fi

}

mkdir -p www
[ ! -f www/test.txt ] && echo "Hello, World!" > www/test.txt
[ ! -f www/index.html ] && echo "<html><body><h1>Test Page</h1></body></html>" > www/index.html
echo "=== Valid Usage ==="
check_response "Request root (index.html)" 200 "" # Should serve index.html by default
check_response "Request existing file" 200 "test.txt"
check_response "HEAD request for existing file" 200 "test.txt" "-I"
check_response "Request non-existing file" 404 "nonexistent.txt"

echo ""
echo "=== Summary ==="
echo "Results: $PASS passed, $FAIL failed"

# Exit with an error code if any tests failed so CI pipelines catch it
[ $FAIL -eq 0 ]
