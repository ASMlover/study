
// single-expression body
var c = 0
while (c < 3) IO.print(c = c + 1)
// expect: 1
// expect: 2
// expect: 3

// block body.
var a = 0
while (a < 3) {
  IO.print(a)
  a = a + 1
}
// expect: 0
// expect: 1
// expect: 2
