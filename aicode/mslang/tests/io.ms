// Test: File I/O module

import "io";

// Test 1: Write to a file
var f = io.open("_test_io.txt", "w");
f.write("hello world\n");
f.write("second line\n");
f.close();
print f.is_open();
// expect: false

// Test 2: Read entire file
var f2 = io.open("_test_io.txt", "r");
var content = f2.read();
f2.close();
print content == "hello world\nsecond line\n";
// expect: true

// Test 3: Read line by line
var f3 = io.open("_test_io.txt", "r");
var line1 = f3.readline();
var line2 = f3.readline();
f3.close();
print line1;
// expect: hello world
print line2;
// expect: second line

// Test 4: Append mode
var f4 = io.open("_test_io.txt", "a");
f4.write("third line\n");
f4.close();

var f5 = io.open("_test_io.txt", "r");
var all = f5.read();
f5.close();
print all == "hello world\nsecond line\nthird line\n";
// expect: true

// Test 5: eof check
var f6 = io.open("_test_io.txt", "r");
print f6.eof();
// expect: false
f6.read();
print f6.eof();
// expect: true
f6.close();

// Test 6: is_open check
var f7 = io.open("_test_io.txt", "r");
print f7.is_open();
// expect: true
f7.close();
print f7.is_open();
// expect: false

// Test 7: module type check
print type(io);
// expect: module
