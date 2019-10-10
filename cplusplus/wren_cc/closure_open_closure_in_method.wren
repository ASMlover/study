
{
  var local = "local"
  class Foo {
    construct new() {}

    method {
      IO.print(local)
    }
  }

  Foo.new().method // expect: local
}
