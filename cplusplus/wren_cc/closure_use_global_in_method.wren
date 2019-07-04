
var global = "global"

class Foo {
  method {
    io.write(global)
  }

  static classMethod {
    io.write(global)
  }
}

Foo.new.method // expect: global
Foo.classMethod // expect: global
