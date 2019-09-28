
class Foo {
  toString { "Foo" }
}

var foo = Foo.new()
IO.print(foo is Foo) // expect: true
IO.print(foo.toString) // expect: Foo
