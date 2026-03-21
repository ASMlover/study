// Test core native functions (15.1)

// --- Math natives ---
print abs(-42)       // expect: 42
print abs(3.14)      // expect: 3.14
print abs(0)         // expect: 0

print ceil(3.2)      // expect: 4
print ceil(-3.2)     // expect: -3

print floor(3.8)     // expect: 3
print floor(-3.8)    // expect: -4

print round(3.5)     // expect: 4
print round(3.4)     // expect: 3
print round(-3.5)    // expect: -4

print sqrt(9)        // expect: 3
print sqrt(0)        // expect: 0
print sqrt(2)        // expect: 1.41421

print pow(2, 10)     // expect: 1024
print pow(3, 0)      // expect: 1

print min(3, 5)      // expect: 3
print min(-1, 1)     // expect: -1

print max(3, 5)      // expect: 5
print max(-1, 1)     // expect: 1

// random() returns a number in [0, 1)
var r = random()
print r >= 0         // expect: true
print r < 1          // expect: true

// --- Conversion natives ---
print int(3.9)       // expect: 3
print int(-3.9)      // expect: -3
print int(true)      // expect: 1
print int(false)     // expect: 0
print int("42")      // expect: 42

print bool(0)        // expect: false
print bool(1)        // expect: true
print bool(nil)      // expect: false
print bool("hello")  // expect: true
print bool("")       // expect: true

// --- I/O natives ---
write_file("_test_io.txt", "hello maple")
print read_file("_test_io.txt")  // expect: hello maple

// --- Assert native ---
assert(true)
assert(1, "should not fail")
print "all passed"   // expect: all passed
