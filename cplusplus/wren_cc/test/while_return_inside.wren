
var f = Function.new {
  while (true) {
    var i = "iii"
    return i
  }
}

IO.print(f.call()) // expect: iii
