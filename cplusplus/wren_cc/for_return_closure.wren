
var f = Function.new {
  for (i in [1, 2, 3]) {
    return Function.new { IO.print(i) }
  }
}

var g = f.call()
g.call() // expect: 1
