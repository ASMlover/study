
var NonLocal = "outer"

{
  var NonLocal = "inner"
  IO.print(NonLocal) // expect: inner
}

IO.print(NonLocal) // expect: outer
