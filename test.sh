#!/usr/bin/env bash
# Improved test script for the http-from-scratch project.

HTTP=./http-from-scratch
LOG_FILE="test.log"
PASS=0
FAIL=0

# Clear previous log
echo "" > "$LOG_FILE"

# Helper to log and check just the exit code
check_response() {
  $HTTP &
  server_PID=$!
  desc=$1
  expected=$2
  sleep 1
  echo -e "\n[TEST]: $desc -> $HTTP $*" >> "$LOG_FILE"

  RESPONSE=$(curl -s -o /dev/null -w "%{http_code}" http://127.0.0.1:8080/)
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

echo "=== Valid Usage ==="
check_response "Start server" 200

echo ""
echo "=== Summary ==="
echo "Results: $PASS passed, $FAIL failed"

# Exit with an error code if any tests failed so CI pipelines catch it
[ $FAIL -eq 0 ]
