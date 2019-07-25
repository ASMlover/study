
var global = "global"

class Foo {
  method {
    IO.print(global)
  }

  static classMethod {
    IO.print(global)
  }
}

(new Foo).method // expect: global
Foo.classMethod // expect: global
