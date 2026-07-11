#!/bin/bash

cd ./tests
gcc -std=c99 *.c -o prf
if [ -f "prf" ]; then
	timeout 20s ./prf foo 2 program1.out > studentout.txt
	if [ $? -eq 0 ] && diff "out1.txt" studentout.txt >/dev/null 2>&1; then
		echo "Test 1: PASS"
	else
		echo "Test 1: FAIL"
	fi

	timeout 20s ./prf rec_foo 1 program2.out > studentout.txt
	if [ $? -eq 0 ] && diff "out2.txt" studentout.txt >/dev/null 2>&1; then
		echo "Test 2: PASS"
	else
		echo "Test 2: FAIL"
	fi

	timeout 20s ./prf rec_bar 1 program3.out > lookout.txt
	if [ $? -eq 0 ] && diff "out3.txt" studentout.txt >/dev/null 2>&1; then
		echo "Test 3: PASS"
	else
		echo "Test 3: FAIL"
	fi

	timeout 20s ./prf food 1 program4.out > studentout.txt
	if [ $? -eq 0 ] && diff "out4.txt" studentout.txt >/dev/null 2>&1; then
		echo "Test 4: PASS"
	else
		echo "Test 4: FAIL"
	fi

	timeout 20s ./prf food 1 program5.out > studentout.txt
	if [ $? -eq 0 ] && diff "out5.txt" studentout.txt >/dev/null 2>&1; then
		echo "Test 5: PASS"
	else
		echo "Test 5: FAIL"
	fi

	timeout 20s ./prf food 1 program6.out > studentout.txt
	if [ $? -eq 0 ] && diff "out6.txt" studentout.txt >/dev/null 2>&1; then
		echo "Test 6: PASS"
	else
		echo "Test 6: FAIL"
	fi

	timeout 20s ./prf foo 2 program7.out > studentout.txt
	if [ $? -eq 0 ] && diff "out7.txt" studentout.txt >/dev/null 2>&1; then
		echo "Test 7: PASS"
	else
		echo "Test 7: FAIL"
	fi

	timeout 20s ./prf foo 2 program8.out > studentout.txt
	if [ $? -eq 0 ] && diff "out8.txt" studentout.txt >/dev/null 2>&1; then
		echo "Test 8: PASS"
	else
		echo "Test 8: FAIL"
	fi

	timeout 20s ./prf foo 2 program9.out > studentout.txt
	if [ $? -eq 0 ] && diff "out9.txt" studentout.txt >/dev/null 2>&1; then
		echo "Test 9: PASS"
	else
		echo "Test 9: FAIL"
	fi

	timeout 20s ./prf foo 1 program10.out > studentout.txt
	if [ $? -eq 0 ] && diff "out10.txt" studentout.txt >/dev/null 2>&1; then
		echo "Test 10: PASS"
	else
		echo "Test 10: FAIL"
	fi

	timeout 20s ./prf foo 2 program11.out R > studentout.txt
	if [ $? -eq 0 ] && diff "out11.txt" studentout.txt >/dev/null 2>&1; then
		echo "Test 11: PASS"
	else
		echo "Test 11: FAIL"
	fi

	timeout 20s ./prf nonexistent 1 program1.out > studentout.txt
	if [ $? -eq 1 ] && diff "out12.txt" studentout.txt >/dev/null 2>&1; then
		echo "Test 12: PASS"
	else
		echo "Test 12: FAIL"
	fi

	timeout 20s ./prf zero_params_func 0 program13.out > studentout.txt
	if [ $? -eq 0 ] && diff "out13.txt" studentout.txt >/dev/null 2>&1; then
		echo "Test 13: PASS"
	else
		echo "Test 13: FAIL"
	fi

	timeout 20s ./prf six_params_func 6 program14.out > studentout.txt
	if [ $? -eq 0 ] && diff "out14.txt" studentout.txt >/dev/null 2>&1; then
		echo "Test 14: PASS"
	else
		echo "Test 14: FAIL"
	fi

	timeout 20s ./prf fib 1 program15.out > studentout.txt
	if [ $? -eq 0 ] && diff "out15.txt" studentout.txt >/dev/null 2>&1; then
		echo "Test 15: PASS"
	else
		echo "Test 15: FAIL"
	fi

	timeout 20s ./prf deep_rec_func 1 program16.out > studentout.txt
	if [ $? -eq 0 ] && diff "out16.txt" studentout.txt >/dev/null 2>&1; then
		echo "Test 16: PASS"
	else
		echo "Test 16: FAIL"
	fi

	timeout 20s ./prf fib 1 program17.out > studentout.txt
	if [ $? -eq 0 ] && diff "out17.txt" studentout.txt >/dev/null 2>&1; then
		echo "Test 17: PASS"
	else
		echo "Test 17: FAIL"
	fi
else
	echo "Compilation error - cannot create debugger file"
fi

rm -f "prf" "studentout.txt"
