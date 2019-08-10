
var f
while (true) {
  var i = "iii"
  f = new Function { IO.print(i) }
  break
}

f.call // expect: iii
