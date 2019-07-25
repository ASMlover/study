
class Foo {
  toString { return "Foo" }
}

var foo = new Foo
IO.print(foo is Foo) // expect: true
IO.print(foo.toString) // expect: Foo
