// Test for-in iterator protocol (12.1)

// --- Iterate over list ---
for (var x in [1, 2, 3]) {
  print x
}
// expect: 1
// expect: 2
// expect: 3

// --- Iterate over range ---
for (var i in range(5)) {
  print i
}
// expect: 0
// expect: 1
// expect: 2
// expect: 3
// expect: 4

// --- Iterate over string (character by character) ---
for (var ch in "abc") {
  print ch
}
// expect: a
// expect: b
// expect: c

// --- Iterate over empty list ---
for (var x in []) {
  print x
}
// (no output)

// --- Iterate with expression ---
var items = [10, 20, 30]
var sum = 0
for (var item in items) {
  sum = sum + item
}
print sum  // expect: 60

// --- Nested for-in ---
for (var i in [1, 2]) {
  for (var j in [10, 20]) {
    print i * 100 + j
  }
}
// expect: 110
// expect: 120
// expect: 210
// expect: 220

// --- Break inside for-in ---
for (var x in [1, 2, 3, 4, 5]) {
  if (x == 3) break
  print x
}
// expect: 1
// expect: 2

// --- Continue inside for-in ---
for (var x in [1, 2, 3, 4, 5]) {
  if (x == 3) continue
  print x
}
// expect: 1
// expect: 2
// expect: 4
// expect: 5

// --- Iterate over map (keys) ---
var m = {"a": 1, "b": 2}
var keys = []
for (var k in m) {
  keys.push(k)
}
print keys.len()  // expect: 2
