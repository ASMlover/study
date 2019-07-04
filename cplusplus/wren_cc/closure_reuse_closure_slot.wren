
{
  var f = nil

  {
    var a = "a"
    f = fn io.write(a)
  }

  {
    var b = "b"
    f.call // expect: a
  }
}
