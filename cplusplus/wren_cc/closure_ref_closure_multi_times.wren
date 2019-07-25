
var f = nil

{
  var a = "a"
  f = fn {
    IO.print(a)
    IO.print(a)
  }
}

f.call
// expect: a
// expect: b
