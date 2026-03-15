// --- integer literals ---

print 42;           // expect: 42
print -7;           // expect: -7
print 0;            // expect: 0

// --- integer arithmetic ---

print 3 + 4;        // expect: 7
print 10 - 3;       // expect: 7
print 6 * 7;        // expect: 42
print 10 % 3;       // expect: 1

// --- integer division always returns float ---

print 10 / 3;       // expect: 3.33333
print 6 / 2;        // expect: 3

// --- auto-promotion: int + float → float ---

print 1 + 2.5;      // expect: 3.5
print 2.5 + 1;      // expect: 3.5
print 10 - 1.5;     // expect: 8.5
print 3 * 1.5;      // expect: 4.5

// --- comparison ---

print 3 > 2;        // expect: true
print 2 < 3;        // expect: true
print 3 == 3;       // expect: true
print 3 != 4;       // expect: true

// --- cross-type comparison ---

print 3 == 3.0;     // expect: true
print 3 != 3.0;     // expect: false
print 3 < 3.5;      // expect: true
print 3 > 2.5;      // expect: true

// --- type() ---

print type(42);      // expect: int
print type(3.14);    // expect: float

// --- int() conversion ---

print int(3.7);      // expect: 3
print int("123");    // expect: 123
print int(true);     // expect: 1
print int(42);       // expect: 42

// --- bitwise AND ---

print 255 & 15;      // expect: 15
print 12 & 10;       // expect: 8

// --- bitwise OR ---

print 12 | 10;       // expect: 14
print 240 | 15;      // expect: 255

// --- bitwise XOR ---

print 12 ^ 10;       // expect: 6
print 255 ^ 255;     // expect: 0

// --- bitwise NOT ---

print ~0;            // expect: -1
print ~(-1);         // expect: 0

// --- left shift ---

print 1 << 8;        // expect: 256
print 3 << 4;        // expect: 48

// --- right shift ---

print 256 >> 8;      // expect: 1
print 48 >> 4;       // expect: 3

// --- combined bitwise ---

print (255 & 240) | 10;     // expect: 250
print 1 << 4 | 1 << 2;     // expect: 20

// --- negation preserves type ---

var a = 5;
print -a;            // expect: -5
print type(-a);      // expect: int

var b = 5.0;
print -b;            // expect: -5
print type(-b);      // expect: float

// --- integer in expressions ---

var x = 10;
var y = 3;
print x + y;         // expect: 13
print x * y;         // expect: 30
print x % y;         // expect: 1

print "done";        // expect: done
