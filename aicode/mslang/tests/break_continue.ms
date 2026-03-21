// Test break in while loop
print "--- break in while loop ---" // expect: --- break in while loop ---
var i = 0
while (i < 10) {
  if (i == 3) break
  i = i + 1
}
print i // expect: 3

// Test continue in while loop
print "--- continue in while loop ---" // expect: --- continue in while loop ---
var sum = 0
var j = 0
while (j < 10) {
  j = j + 1
  if (j == 5) continue
  sum = sum + j
}
print sum // expect: 50

// Test break in for loop
print "--- break in for loop ---" // expect: --- break in for loop ---
var k = 0
for (var x = 0; x < 10; x = x + 1) {
  if (x == 4) break
  k = x
}
print k // expect: 3

// Test continue in for loop
print "--- continue in for loop ---" // expect: --- continue in for loop ---
var total = 0
for (var x = 0; x < 6; x = x + 1) {
  if (x == 3) continue
  total = total + x
}
print total // expect: 12

// Test nested loops with break
print "--- nested loops with break ---" // expect: --- nested loops with break ---
var result = 0
for (var a = 0; a < 3; a = a + 1) {
  for (var b = 0; b < 3; b = b + 1) {
    if (b == 1) break
    result = result + 1
  }
}
print result // expect: 3

// Test nested loops with continue
print "--- nested loops with continue ---" // expect: --- nested loops with continue ---
var count = 0
for (var a = 0; a < 3; a = a + 1) {
  for (var b = 0; b < 3; b = b + 1) {
    if (b == 1) continue
    count = count + 1
  }
}
print count // expect: 6

// Test break with local variables in loop body
print "--- break with locals ---" // expect: --- break with locals ---
var found = 0
for (var n = 0; n < 10; n = n + 1) {
  var temp = n * 2
  if (temp == 8) {
    found = n
    break
  }
}
print found // expect: 4
