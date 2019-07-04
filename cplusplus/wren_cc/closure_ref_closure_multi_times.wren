
var f = nil

{
  var a = "a"
  f = fn {
    io.write(a)
    io.write(a)
  }
}

f.call
// expect: a
// expect: b
