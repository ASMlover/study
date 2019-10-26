
// create a local scope for `then` expressIO.
var a = "out"
if (true) {
  var a = "in"
}
IO.print(a) // expect: out

// create a local scope for `else` expressIO.
var b = "out"
if (false) "dummy" else {
  var b = "in"
}
IO.print(b) // expect: out
