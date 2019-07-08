
{
  var a = "outer"
  {
    var a = a + " inner"
    io.write(a) // expect: outer inner
  }
}
