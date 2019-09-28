
var f = Function.new {
  while (true) {
    var i = "iii"
    return Function.new { IO.print(i) }
  }
}

var g = f.call()
g.call() // expect: iii
