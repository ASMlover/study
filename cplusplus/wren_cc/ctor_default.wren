
class Foo {
  toString { return "Foo" }
}

var foo = new Foo
io.write(foo is Foo) // expect: true
io.write(foo.toString) // expect: Foo
