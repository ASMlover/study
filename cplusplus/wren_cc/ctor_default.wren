
class Foo {
  toString { return "Foo" }
}

var foo = Foo.new
io.write(foo is Foo) // expect: true
io.write(foo.toString) // expect: Foo
