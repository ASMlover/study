// Basic ternary operator
print true ? "yes" : "no"      // expect: yes
print false ? "yes" : "no"     // expect: no

// Ternary with expressions
print 1 + 1 == 2 ? "math" : "broken" // expect: math

// Ternary with variables
var a = 10
var b = 20
var max = a > b ? a : b
print max                       // expect: 20

// Nested ternary
var x = 3
print x == 1 ? "one" : x == 2 ? "two" : "other" // expect: other

// Ternary in expression context
print "result: " + (true ? "ok" : "fail") // expect: result: ok

// Ternary with nil
print nil ? "truthy" : "falsy" // expect: falsy

// Ternary with number 0 (0 is falsy in Maple)
print 0 ? "truthy" : "falsy"  // expect: falsy
