// Constant folding: numeric binary operations
print 1 + 2;            // expect: 3
print 10 - 3;           // expect: 7
print 3 * 4;            // expect: 12
print 15 / 3;           // expect: 5
print 10 % 3;           // expect: 1

// Constant folding: chained expressions
print 1 + 2 * 3;        // expect: 7
print (2 + 3) * 4;      // expect: 20
print 10 - 2 * 3 + 1;   // expect: 5
print 100 / 5 / 4;      // expect: 5

// Constant folding: unary negation
print -5;               // expect: -5
print -(-3);            // expect: 3
print -(2 + 3);         // expect: -5

// Constant folding: floating point
print 1.5 + 2.5;        // expect: 4
print 3.14 * 2;         // expect: 6.28
print 10.0 / 3.0;       // expect: 3.33333

// Constant folding: string concatenation
print "hello" + " " + "world";  // expect: hello world
print "abc" + "def";            // expect: abcdef

// Mixed: constant + variable (should NOT fold, but still work)
var x = 10;
print x + 5;            // expect: 15
print 5 + x;            // expect: 15

// Division by zero should NOT fold (let VM handle it)
// print 1 / 0;  // would be runtime error

// Modulo by zero should NOT fold
// print 1 % 0;  // would be runtime error

// Constant folding with grouping
print (1 + 2) * (3 + 4);  // expect: 21
print -(1 + 2);            // expect: -3
