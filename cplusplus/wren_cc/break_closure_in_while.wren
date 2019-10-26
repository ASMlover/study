
var f
while (true) {
  var i = "iii"
  f = Function.new { IO.print(i) }
  break
}

f.call() // expect: iii
