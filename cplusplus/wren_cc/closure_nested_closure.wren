
var f = nil

new Function {
  var a = "a"
  new Function {
    var b = "b"
    new Function {
      var c = "c"
      f = new Function {
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
