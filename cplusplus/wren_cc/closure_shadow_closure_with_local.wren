
{
  var foo = "closure"

  new Function {
    {
      IO.print(foo) // expect: closure
      var foo = "shadow"
      IO.print(foo) // expect: shadow
    }
    IO.print(foo) // expect: closure
  }.call
}
