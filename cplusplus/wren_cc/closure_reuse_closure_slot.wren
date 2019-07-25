
{
  var f = nil

  {
    var a = "a"
    f = fn IO.print(a)
  }

  {
    var b = "b"
    f.call // expect: a
  }
}
