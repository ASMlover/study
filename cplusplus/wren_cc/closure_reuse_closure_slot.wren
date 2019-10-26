
{
  var f = nil

  {
    var a = "a"
    f = Function.new { IO.print(a) }
  }

  {
    var b = "b"
    f.call() // expect: a
  }
}
