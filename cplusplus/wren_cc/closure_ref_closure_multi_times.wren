
var f = nil

{
  var a = "a"
  f = Function.new {
    IO.print(a)
    IO.print(a)
  }
}

f.call()
// expect: a
// expect: b
