
var f = new Function {
  for (i in [1, 2, 3]) {
    return i
  }
}

IO.print(f.call()) // expect: 1
