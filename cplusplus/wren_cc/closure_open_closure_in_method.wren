
{
  var local = "local"
  class Foo {
    method {
      IO.print(local)
    }
  }

  Foo.new().method // expect: local
}
