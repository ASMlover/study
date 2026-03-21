// Test: control flow paren suppression (no ASI after condition `)`)

// --- simple if ---
print "--- simple if ---" // expect: --- simple if ---
if (true)
  print "if body" // expect: if body

if (false)
  print "should not print"

// --- if-else ---
print "--- if-else ---" // expect: --- if-else ---
if (true)
  print "if branch" // expect: if branch
else
  print "else branch"

if (false)
  print "if branch"
else
  print "else branch" // expect: else branch

// --- if-else if-else ---
print "--- if-else if-else ---" // expect: --- if-else if-else ---
var x = 2
if (x == 1)
  print "one"
else if (x == 2)
  print "two" // expect: two
else
  print "other"

// --- nested if ---
print "--- nested if ---" // expect: --- nested if ---
if (true)
  if (true)
    print "nested true" // expect: nested true

if (true)
  if (false)
    print "inner false"
  else
    print "inner else" // expect: inner else

// --- if with block ---
print "--- if with block ---" // expect: --- if with block ---
if (true) {
  var a = 10
  print a // expect: 10
}

if (false) {
  print "nope"
} else {
  print "else block" // expect: else block
}

// --- nested parens in condition ---
print "--- nested parens ---" // expect: --- nested parens ---
fun square(n) {
  return n * n
}
if (square(3) > 8)
  print "9 > 8" // expect: 9 > 8

if (square(2) + square(3) == 13)
  print "4 + 9 = 13" // expect: 4 + 9 = 13

// --- multi-line condition ---
print "--- multi-line condition ---" // expect: --- multi-line condition ---
if (true and
  true)
  print "multi-line and" // expect: multi-line and

if (1 > 0 and
  2 > 1 and
  3 > 2)
  print "chained and" // expect: chained and

// --- while loop ---
print "--- while ---" // expect: --- while ---
var i = 0
while (i < 3)
  i += 1
print i // expect: 3

var j = 0
while (j < 3) {
  j += 1
}
print j // expect: 3

// --- for loop ---
print "--- for ---" // expect: --- for ---
var sum = 0
for (var k = 1; k <= 5; k += 1)
  sum += k
print sum // expect: 15

for (var k = 0; k < 3; k += 1) {
  print k // expect: 0
} // expect: 1
// expect: 2

// --- for inside if ---
print "--- for in if ---" // expect: --- for in if ---
if (true) {
  var total = 0
  for (var n = 1; n <= 4; n += 1)
    total += n
  print total // expect: 10
}

// --- if inside for ---
print "--- if in for ---" // expect: --- if in for ---
for (var n = 1; n <= 5; n += 1)
  if (n % 2 == 0)
    print n // expect: 2
// expect: 4

// --- switch ---
print "--- switch ---" // expect: --- switch ---
switch (1 + 1) {
  case 2:
    print "matched 2" // expect: matched 2
}

var val = "hello"
switch (val) {
  case "hello":
    print "greeting" // expect: greeting
  case "bye":
    print "farewell"
}

// --- blank lines between condition and body ---
print "--- blank lines ---" // expect: --- blank lines ---
if (true)

  print "after blank" // expect: after blank

// --- while with nested condition parens ---
print "--- while nested ---" // expect: --- while nested ---
var m = 0
while ((m + 1) < 4)
  m += 1
print m // expect: 3
