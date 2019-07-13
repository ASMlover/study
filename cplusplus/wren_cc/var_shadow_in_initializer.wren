
{
  var a = "outer"
  {
    var a = a + " inner"
    IO.write(a) // expect: outer inner
  }
}
