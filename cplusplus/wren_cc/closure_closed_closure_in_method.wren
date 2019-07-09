
var f = nil

{
  var local = "local"
  class Foo {
    method {
      io.write(local)
    }
  }

  f = new Foo
}
f.method // expect: local
