
var f = nil

{
  var local = "local"
  class Foo {
    method {
      IO.print(local)
    }
  }

  f = Foo.new()
}
f.method // expect: local
