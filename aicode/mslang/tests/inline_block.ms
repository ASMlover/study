// Test: statements in inline blocks should not require semicolons

// Simple inline if block
var x = 10
if (x > 5) { print x }
// expect: 10

// Inline while block with multiple stmts (still need ; between them)
var i = 0
while (i < 3) { print i; i = i + 1 }
// expect: 0
// expect: 1
// expect: 2

// Inline for block
var sum = 0
for (var j = 0; j < 3; j = j + 1) { sum = sum + j }
print sum
// expect: 3

// Inline function body
fun double(n) { return n * 2 }
print double(7)
// expect: 14

// Expression statement without trailing semicolon before }
fun greet(name) { print "hi " + name }
greet("world")
// expect: hi world
