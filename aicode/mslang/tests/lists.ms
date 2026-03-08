// List literal and print
var a = [1, 2, 3];
print a;             // expect: [1, 2, 3]

// Empty list
var empty = [];
print empty;         // expect: []

// Index get
print a[0];          // expect: 1
print a[1];          // expect: 2
print a[2];          // expect: 3

// Index set
a[1] = 42;
print a[1];          // expect: 42
print a;             // expect: [1, 42, 3]

// List methods
print a.len();       // expect: 3

a.push(4);
print a;             // expect: [1, 42, 3, 4]
print a.len();       // expect: 4

var last = a.pop();
print last;          // expect: 4
print a;             // expect: [1, 42, 3]
print a.len();       // expect: 3

// Mixed types in list
var mixed = [1, "hello", true, nil];
print mixed;         // expect: [1, "hello", true, nil]
print mixed[1];      // expect: hello
print mixed.len();   // expect: 4

// Nested lists
var nested = [[1, 2], [3, 4]];
print nested;        // expect: [[1, 2], [3, 4]]
print nested[0];     // expect: [1, 2]
print nested[1][0];  // expect: 3

// Type check
print type(a);       // expect: list
print type([]);      // expect: list

// String indexing
var s = "hello";
print s[0];          // expect: h
print s[4];          // expect: o

// List in expressions
var b = [10, 20, 30];
print b[0] + b[1];  // expect: 30

// Single element list
var single = [99];
print single;        // expect: [99]
print single[0];     // expect: 99
