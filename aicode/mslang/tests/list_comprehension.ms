// List comprehension tests

// Basic comprehension with range
var squares = [x * x for x in range(5)]
print squares          // expect: [0, 1, 4, 9, 16]

// Comprehension with list literal
var doubled = [x * 2 for x in [1, 2, 3]]
print doubled          // expect: [2, 4, 6]

// Comprehension with filter
var evens = [x for x in range(10) if x % 2 == 0]
print evens            // expect: [0, 2, 4, 6, 8]

// Comprehension with expression and filter
var big_squares = [x * x for x in range(10) if x > 5]
print big_squares      // expect: [36, 49, 64, 81]

// Comprehension with string iteration
var chars = [ch for ch in "hello"]
print chars            // expect: ["h", "e", "l", "l", "o"]

// Nested expression
var result = [x + 1 for x in [10, 20, 30]]
print result           // expect: [11, 21, 31]

// Comprehension result used in expression
print [x for x in range(3)].len() // expect: 3

// Empty result with filter
var empty = [x for x in range(5) if x > 10]
print empty            // expect: []

// Normal list literals still work
var normal = [1, 2, 3]
print normal           // expect: [1, 2, 3]

// Single-element list still works
var single = [42]
print single           // expect: [42]

// Empty list still works
var empty2 = []
print empty2           // expect: []
