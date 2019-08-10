
var f = new Function {
  for (i in [1, 2, 3]) {
    return new Function { IO.print(i) }
  }
}

var g = f.call
g.call // expect: 1
