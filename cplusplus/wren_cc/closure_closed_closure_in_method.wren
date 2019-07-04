
var f = nil

{
  var local = "local"
  class Foo {
    method {
      io.write(local)
    }
  }

  f = Foo.new
}
f.method // expect: local
