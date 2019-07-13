
class Foo {
  toString { return "Foo" }
}

var foo = new Foo
IO.write(foo is Foo) // expect: true
IO.write(foo.toString) // expect: Foo
