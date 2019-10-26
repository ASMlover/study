
var f = nil

Function.new {
  var a = "a"
  Function.new {
    var b = "b"
    Function.new {
      var c = "c"
      f = Function.new {
        IO.print(a)
        IO.print(b)
        IO.print(c)
      }
    }.call()
  }.call()
}.call()

f.call()
// expect: a
// expect: b
// expect: c
