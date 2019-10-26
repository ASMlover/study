
class Foo {
  static write { IO.print(__field) }
  static init { __field = "value" }
}

Foo.init
Foo.write // expect: value
