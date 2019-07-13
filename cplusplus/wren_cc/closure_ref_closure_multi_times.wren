
var f = nil

{
  var a = "a"
  f = fn {
    IO.write(a)
    IO.write(a)
  }
}

f.call
// expect: a
// expect: b
