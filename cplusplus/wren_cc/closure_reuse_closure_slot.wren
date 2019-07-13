
{
  var f = nil

  {
    var a = "a"
    f = fn IO.write(a)
  }

  {
    var b = "b"
    f.call // expect: a
  }
}
