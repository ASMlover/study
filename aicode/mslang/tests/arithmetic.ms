// Basic arithmetic operations
print 1 + 2;        // expect: 3
print 5 - 3;        // expect: 2
print 2 * 4;        // expect: 8
print 10 / 2;       // expect: 5
print -5;           // expect: -5

// Precedence
print 2 + 3 * 4;    // expect: 14
print (2 + 3) * 4;  // expect: 20
print 10 - 2 * 3;   // expect: 4
print 8 / 2 + 1;    // expect: 5

// Floating point
print 3.14;         // expect: 3.14
print 1.5 + 2.5;    // expect: 4

// Comparison
print 1 < 2;        // expect: true
print 3 > 5;        // expect: false
print 2 <= 2;       // expect: true
print 4 >= 5;       // expect: false
print 1 == 1;       // expect: true
print 1 != 2;       // expect: true

// Boolean operations
print !true;         // expect: false
print !false;        // expect: true
print !!true;        // expect: true
