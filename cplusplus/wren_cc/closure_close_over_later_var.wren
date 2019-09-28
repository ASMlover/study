
Function.new {
  var a = "a"
  var b = "b"
  Function.new {
    IO.print(b) // expect: b
    IO.print(a) // expect: a
  }.call()
}.call()
