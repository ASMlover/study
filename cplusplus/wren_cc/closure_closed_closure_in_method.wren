
var f = nil

{
  var local = "local"
  class Foo {
    method {
      IO.print(local)
    }
  }

  f = new Foo
}
f.method // expect: local
