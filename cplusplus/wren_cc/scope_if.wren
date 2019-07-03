
// create a local scope for `then` expression
var a = "out"
if (true) {
  var a = "in"
}
io.write(a) // expect: out

// create a local scope for `else` expression
var b = "out"
if (false) "dummy" else {
  var b = "in"
}
io.write(b) // expect: out
