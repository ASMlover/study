
var f = new Function {
  while (true) {
    var i = "iii"
    return new Function { IO.print(i) }
  }
}

var g = f.call()
g.call() // expect: iii
