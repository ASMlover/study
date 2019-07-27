
var f = fn {
  while (true) {
    var i = "iii"
    return fn IO.print(i)
  }
}

var g = f.call
g.call // expect: iii
