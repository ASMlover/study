
{
  var f = nil

  {
    var a = "a"
    f = new Function { IO.print(a) }
  }

  {
    var b = "b"
    f.call // expect: a
  }
}
