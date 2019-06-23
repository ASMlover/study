
// single-expression body
var a = 0
while (a < 3) io.write(a = a + 1)
// expect: 1
// expect: 2
// expect: 3

// block body
var b = 0
while (b < 3) {
  io.write(b)
  b = b + 1
}
// expect: 0
// expect: 1
// expect: 2

// newline after `while`
var c = 0
while
(c < 3) io.write(c = c + 1)
// expect: 1
// expect: 2
// expect: 3

// result is nil
var d = 0
var e = while (d < 3) { d = d + 1}
io.write(e) // expect: nil

// definition body
while (false) var a = "ok"
while (false) class Foo {}
