
var f = nil

{
  var a = "a"
  f = new Function {
    IO.print(a)
    IO.print(a)
  }
}

f.call
// expect: a
// expect: b
