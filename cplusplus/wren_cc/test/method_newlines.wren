
class Foo {
  construct new() {}
  method(a, b) { "method " + a + " " + b }
  [a, b] { "subscript " + a + " " + b }
}

var foo = Foo.new()

IO.print(foo.method("a",

  "b"

)) // expect: method a b

IO.print(foo["a",

  "b"

]) // expect: subscript a b
