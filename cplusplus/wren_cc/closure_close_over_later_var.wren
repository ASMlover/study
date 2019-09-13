
new Function {
  var a = "a"
  var b = "b"
  new Function {
    IO.print(b) // expect: b
    IO.print(a) // expect: a
  }.call()
}.call()
