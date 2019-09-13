
{
  var local = "local"
  new Function {
    IO.print(local) // expect: local
  }.call()
}
