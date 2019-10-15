
var closure

{
  var a = "a"

  {
    var b = "b"
    closure = Function.new { a }
    if (false) Function.new { b }
  }

  IO.print(closure.call()) // expect: a
}
