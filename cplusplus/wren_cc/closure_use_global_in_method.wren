
var global = "global"

class Foo {
  method {
    IO.write(global)
  }

  static classMethod {
    IO.write(global)
  }
}

(new Foo).method // expect: global
Foo.classMethod // expect: global
