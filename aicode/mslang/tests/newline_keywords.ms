// Test: return/break/continue ASI (bare line vs same-line value)

// --- return with value on same line ---
print "--- return value ---" // expect: --- return value ---
fun add(a, b) {
  return a + b
}
print add(3, 4) // expect: 7

fun greet() {
  return "hello"
}
print greet() // expect: hello

// --- bare return (ASI inserts ; → returns nil) ---
print "--- bare return ---" // expect: --- bare return ---
fun bare_return() {
  return
}
print bare_return() // expect: nil

fun bare_return2() {
  return
  42
}
print bare_return2() // expect: nil

// --- return in if ---
print "--- return in if ---" // expect: --- return in if ---
fun abs(n) {
  if (n < 0)
    return -n
  return n
}
print abs(-5) // expect: 5
print abs(3) // expect: 3

// --- return from nested block ---
print "--- return nested ---" // expect: --- return nested ---
fun find_first_even(list) {
  for (var i = 0; i < list.len(); i += 1) {
    if (list[i] % 2 == 0)
      return list[i]
  }
  return -1
}
print find_first_even([1, 3, 4, 5]) // expect: 4
print find_first_even([1, 3, 5]) // expect: -1

// --- break in while ---
print "--- break while ---" // expect: --- break while ---
var i = 0
while (true) {
  if (i >= 3)
    break
  i += 1
}
print i // expect: 3

// --- break in for ---
print "--- break for ---" // expect: --- break for ---
var sum = 0
for (var k = 1; k <= 100; k += 1) {
  if (k > 5)
    break
  sum += k
}
print sum // expect: 15

// --- continue in while ---
print "--- continue while ---" // expect: --- continue while ---
var total = 0
var j = 0
while (j < 10) {
  j += 1
  if (j % 2 != 0)
    continue
  total += j
}
print total // expect: 30

// --- continue in for ---
print "--- continue for ---" // expect: --- continue for ---
var odds = 0
for (var n = 1; n <= 10; n += 1) {
  if (n % 2 == 0)
    continue
  odds += n
}
print odds // expect: 25

// --- break and continue together ---
print "--- break + continue ---" // expect: --- break + continue ---
var result = 0
for (var n = 1; n <= 20; n += 1) {
  if (n % 3 == 0)
    continue
  if (n > 10)
    break
  result += n
}
print result // expect: 37
