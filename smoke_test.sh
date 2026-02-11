#!/bin/bash

# ============================================================
#  mysh MASTER TEST SUITE (Final Version)
#  Covers: Basic, Redirection, Background, Edge Cases, Buffers, & Syntax
# ============================================================

GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

EXECUTABLE="./mysh"
TEST_FILE="test_data.txt"
OUT_FILE="test_out.txt"
LOCKED_FILE="test_locked.txt"

# Cleanup
rm -f $TEST_FILE $OUT_FILE $LOCKED_FILE

echo "------------------------------------------------------------"
echo "🚀 STARTING MASTER TEST SUITE (17 Checks)"
echo "------------------------------------------------------------"

if [ ! -x "$EXECUTABLE" ]; then
    echo -e "${RED}FAILED${NC}: $EXECUTABLE not found. Run 'make'."
    exit 1
fi

# --- BASIC & BUILTINS ---

echo -n "[01/17] Basic Execution (ls)... "
if echo "ls" | $EXECUTABLE > /dev/null 2>&1; then echo -e "${GREEN}PASSED${NC}"; else echo -e "${RED}FAILED${NC}"; fi

echo -n "[02/17] Built-in (cd)... "
if [[ $(echo -e "cd /tmp\npwd" | $EXECUTABLE) == *"/tmp"* ]]; then echo -e "${GREEN}PASSED${NC}"; else echo -e "${RED}FAILED${NC}"; fi

echo -n "[03/17] Whitespace Torture (ls   -l)... "
if echo -e "ls \t  -l" | $EXECUTABLE > /dev/null 2>&1; then echo -e "${GREEN}PASSED${NC}"; else echo -e "${RED}FAILED${NC}"; fi

echo -n "[04/17] Empty Input (Enter key)... "
if echo -e "\n\n\nls" | $EXECUTABLE > /dev/null 2>&1; then echo -e "${GREEN}PASSED${NC}"; else echo -e "${RED}FAILED${NC}"; fi

# --- REDIRECTION ---

echo -n "[05/17] Output Redirection (>)... "
echo "DATA" > $TEST_FILE
echo "cat $TEST_FILE > $OUT_FILE" | $EXECUTABLE > /dev/null 2>&1
if grep -q "DATA" $OUT_FILE; then echo -e "${GREEN}PASSED${NC}"; else echo -e "${RED}FAILED${NC}"; fi

echo -n "[06/17] Append Redirection (>>)... "
echo "Line1" > $OUT_FILE
echo "echo Line2 >> $OUT_FILE" | $EXECUTABLE > /dev/null 2>&1
if [ $(wc -l < $OUT_FILE) -eq 2 ]; then echo -e "${GREEN}PASSED${NC}"; else echo -e "${RED}FAILED${NC}"; fi

echo -n "[07/17] Input Safety (<)... "
echo "IMPORTANT" > $TEST_FILE
echo "cat < $TEST_FILE" | $EXECUTABLE > /dev/null 2>&1
if grep -q "IMPORTANT" $TEST_FILE; then echo -e "${GREEN}PASSED${NC}"; else echo -e "${RED}FAILED (File Wiped!)${NC}"; fi

echo -n "[08/17] Reverse Redirection (> before <)... "
echo "REV" > $TEST_FILE
echo "cat > $OUT_FILE < $TEST_FILE" | $EXECUTABLE > /dev/null 2>&1
if grep -q "REV" $OUT_FILE; then echo -e "${GREEN}PASSED${NC}"; else echo -e "${RED}FAILED${NC}"; fi

# --- ERROR HANDLING ---

echo -n "[09/17] Non-Existent Command... "
$EXECUTABLE <<EOF > /dev/null 2> error_log.txt
thiscommanddoesnotexist
exit
EOF
if [ -s error_log.txt ]; then echo -e "${GREEN}PASSED${NC}"; else echo -e "${RED}FAILED (No error printed)${NC}"; fi

echo -n "[10/17] Permission Denied Error... "
touch $LOCKED_FILE && chmod 444 $LOCKED_FILE
$EXECUTABLE <<EOF > /dev/null 2> error_log.txt
echo "hack" > $LOCKED_FILE
exit
EOF
if grep -q -i "denied" error_log.txt || grep -q -i "permission" error_log.txt; then echo -e "${GREEN}PASSED${NC}"; else echo -e "${RED}FAILED${NC}"; fi

echo -n "[11/17] Missing Input File... "
# We try to read a file that doesn't exist. Shell should NOT crash.
$EXECUTABLE <<EOF > /dev/null 2> error_log.txt
cat < ghost_file_xyz.txt
exit
EOF
# It passed if the shell exited with 0 (didn't crash) AND printed an error
if [ $? -eq 0 ] && [ -s error_log.txt ]; then echo -e "${GREEN}PASSED${NC}"; else echo -e "${RED}FAILED (Crashed or no error)${NC}"; fi

# --- BACKGROUND JOBS ---

echo -n "[12/17] Background with IO (echo > &)... "
rm -f $OUT_FILE
echo "echo BG_TEST > $OUT_FILE &" | $EXECUTABLE > /dev/null 2>&1
sleep 1
if grep -q "BG_TEST" $OUT_FILE; then echo -e "${GREEN}PASSED${NC}"; else echo -e "${RED}FAILED${NC}"; fi

echo -n "[13/17] Multiple Concurrent BG Jobs... "
start=$(date +%s)
$EXECUTABLE <<EOF > /dev/null 2>&1
sleep 2 &
sleep 2 &
sleep 2 &
exit
EOF
end=$(date +%s)
if [ $((end-start)) -lt 4 ]; then echo -e "${GREEN}PASSED${NC}"; else echo -e "${RED}FAILED (Not Parallel)${NC}"; fi

# --- STRESS & BUFFERS ---

echo -n "[14/17] Argument Overload (200 args)... "
args=$(seq -s " " 1 200)
echo "echo $args" | $EXECUTABLE > $OUT_FILE 2>&1
if grep -q "200" $OUT_FILE; then echo -e "${GREEN}PASSED${NC}"; else echo -e "${RED}FAILED${NC}"; fi

echo -n "[15/17] Buffer Overflow Prevention... "
long_str=$(printf 'a%.0s' {1..2000})
$EXECUTABLE <<EOF > /dev/null 2>&1
echo $long_str
exit
EOF
if [ $? -eq 0 ]; then echo -e "${GREEN}PASSED${NC}"; else echo -e "${RED}FAILED (Crashed/Segfault)${NC}"; fi

echo -n "[16/17] Exit with Garbage Args... "
if echo "exit please now" | $EXECUTABLE > /dev/null 2>&1; then echo -e "${GREEN}PASSED${NC}"; else echo -e "${RED}FAILED${NC}"; fi

# --- SYNTAX EDGE CASES ---

echo -n "[17/17] Syntax Error (ls >)... "
# This checks if your parser handles NULL tokens correctly
$EXECUTABLE <<EOF > /dev/null 2> error_log.txt
ls >
exit
EOF
# Should print "syntax error" and NOT crash
if [ $? -eq 0 ] && grep -q "syntax error" error_log.txt; then echo -e "${GREEN}PASSED${NC}"; else echo -e "${RED}FAILED (Crashed or no error msg)${NC}"; fi


# Cleanup
rm -f $TEST_FILE $OUT_FILE $LOCKED_FILE error_log.txt
echo "------------------------------------------------------------"
echo "🎉 MASTER TEST COMPLETE"