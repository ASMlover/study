
var f
for (i in [1, 2, 3]) {
  var j = 4
  f = Function.new { IO.print(i + j) }
  break
}

f.call() // expect: 5
