// Test: multi-line expression continuation (no ASI on non-trigger tokens)

// --- binary operator continuation ---
print "--- operator continuation ---" // expect: --- operator continuation ---
var x = 1 +
  2
print x // expect: 3

var y = 10 -
  3 +
  2
print y // expect: 9

var z = 2 *
  3 +
  4
print z // expect: 10

var w = 100 /
  5
print w // expect: 20

var m = 10 %
  3
print m // expect: 1

// --- comparison / equality continuation ---
print "--- comparison continuation ---" // expect: --- comparison continuation ---
var a = 10 ==
  10
print a // expect: true

var b = 10 !=
  5
print b // expect: true

var c = 10 >=
  5
print c // expect: true

var d = 3 <=
  7
print d // expect: true

var e = 3 <
  7
print e // expect: true

var f = 10 >
  5
print f // expect: true

// --- logical operator continuation ---
print "--- logical continuation ---" // expect: --- logical continuation ---
var p = true and
  true
print p // expect: true

var q = false or
  true
print q // expect: true

// --- assignment operator continuation ---
print "--- assignment continuation ---" // expect: --- assignment continuation ---
var v =
  42
print v // expect: 42

var v2 = 10
v2 +=
  5
print v2 // expect: 15

v2 -=
  3
print v2 // expect: 12

v2 *=
  2
print v2 // expect: 24

v2 /=
  4
print v2 // expect: 6

// --- ternary operator continuation ---
print "--- ternary continuation ---" // expect: --- ternary continuation ---
var t = true ?
  "yes" :
  "no"
print t // expect: yes

var t2 = false ?
  "yes" :
  "no"
print t2 // expect: no

// --- comma continuation (function arguments) ---
print "--- comma continuation ---" // expect: --- comma continuation ---
fun add3(a, b, c) {
  return a + b + c
}
print add3(1,
  2,
  3) // expect: 6

print add3(
  10,
  20,
  30) // expect: 60

// --- comma continuation (function parameters) ---
print "--- param continuation ---" // expect: --- param continuation ---
fun multi_param(a,
  b,
  c) {
  return a * b * c
}
print multi_param(2, 3, 4) // expect: 24

// --- comma continuation (list literals) ---
print "--- list continuation ---" // expect: --- list continuation ---
var list = [1, 2,
  3, 4,
  5]
print list // expect: [1, 2, 3, 4, 5]

var list2 = [
  10,
  20,
  30]
print list2 // expect: [10, 20, 30]

// --- comma continuation (map literals) ---
print "--- map continuation ---" // expect: --- map continuation ---
var map = {"a": 1,
  "b": 2,
  "c": 3}
print map["a"] // expect: 1
print map["b"] // expect: 2

// --- dot / method chaining (same line) ---
print "--- method calls ---" // expect: --- method calls ---
var s = "hello"
print s.len() // expect: 5
print s.upper() // expect: HELLO

// --- parenthesized expression continuation ---
print "--- paren continuation ---" // expect: --- paren continuation ---
var r = 10 * (
  2 + 3)
print r // expect: 50

// --- nested multi-line ---
print "--- nested multi-line ---" // expect: --- nested multi-line ---
var nested = add3(1 +
    2,
  3 *
    4,
  5)
print nested // expect: 20
