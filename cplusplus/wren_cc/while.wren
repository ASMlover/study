
var a = 0
while (a < 3) {
  IO.write(a)
  a = a + 1
}
// expect: 0
// expect: 1
// expect: 2

// newline after `while`
var c = 0
while
(c < 3) IO.write(c = c + 1)
// expect: 1
// expect: 2
// expect: 3
