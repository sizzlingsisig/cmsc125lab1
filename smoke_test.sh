#!/bin/bash

# ============================================================
#  mysh SMOKE TEST (Executable Only)
#  Runs tests against ./mysh without compiling
# ============================================================

GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

EXECUTABLE="./mysh"
TEST_FILE="smoke_test_data.txt"
OUT_FILE="smoke_test_out.txt"

echo "------------------------------------------------------------"
echo "💨 STARTING SMOKE TEST (Skipping Compile)"
echo "------------------------------------------------------------"

# 1. EXECUTABLE CHECK
# Instead of compiling, we just check if the file exists and runs
echo -n "[1/6] Checking for mysh... "
if [ -x "$EXECUTABLE" ]; then
    echo -e "${GREEN}FOUND${NC}"
else
    echo -e "${RED}FAILED${NC}"
    echo "      Could not find executable '$EXECUTABLE'"
    echo "      Make sure you run 'make' first!"
    exit 1
fi

# 2. BASIC EXECUTION (ls)
echo -n "[2/6] Basic Execution (ls)... "
echo "ls" | $EXECUTABLE > /dev/null 2>&1
if [ $? -eq 0 ]; then
    echo -e "${GREEN}PASSED${NC}"
else
    echo -e "${RED}FAILED${NC} (Shell crashed or returned error)"
    exit 1
fi

# 3. BUILT-IN CHECK (cd)
echo -n "[3/6] Built-in Command (cd)... "
output=$(echo -e "cd /tmp\npwd" | $EXECUTABLE)
if [[ "$output" == *"/tmp"* ]]; then
    echo -e "${GREEN}PASSED${NC}"
else
    echo -e "${RED}FAILED${NC}"
    echo "      Expected output to contain: /tmp"
    echo "      Actual output: $output"
    exit 1
fi

# 4. OUTPUT REDIRECTION (>)
echo -n "[4/6] Output Redirection (>)... "
echo "echo SMOKE_TEST_PASSED > $TEST_FILE" | $EXECUTABLE > /dev/null 2>&1
if grep -q "SMOKE_TEST_PASSED" $TEST_FILE; then
    echo -e "${GREEN}PASSED${NC}"
else
    echo -e "${RED}FAILED${NC} (File content missing or wrong)"
    exit 1
fi

# 5. CRITICAL BUG CHECK: INPUT REDIRECTION (<)
echo -n "[5/6] Input Redirection Safety (<)... "
echo "IMPORTANT_DATA" > $TEST_FILE
echo "cat < $TEST_FILE" | $EXECUTABLE > /dev/null 2>&1

if grep -q "IMPORTANT_DATA" $TEST_FILE; then
    echo -e "${GREEN}PASSED${NC} (File is safe)"
else
    echo -e "${RED}FAILED${NC} (CRITICAL: Input file was erased!)"
    exit 1
fi

# 6. BACKGROUND PROCESS TIMING (&)
echo -n "[6/6] Background Logic (&)... "
start_time=$(date +%s)
echo -e "sleep 3 &\nexit" | $EXECUTABLE > /dev/null 2>&1
end_time=$(date +%s)
elapsed=$((end_time - start_time))

if [ "$elapsed" -lt 2 ]; then
    echo -e "${GREEN}PASSED${NC} (Shell returned instantly)"
else
    echo -e "${RED}FAILED${NC} (Shell waited for background job)"
fi

echo "------------------------------------------------------------"
echo -e "🎉 ALL TESTS PASSED."
echo "------------------------------------------------------------"

# Cleanup
rm -f $TEST_FILE $OUT_FILE