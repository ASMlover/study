
var a = "out"
var i = 0
while ((i = i + 1) <= 1) {
  var a = "in"
}
IO.print(a) // expect: out
