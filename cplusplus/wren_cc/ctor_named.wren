
class Foo {
  this bar { io.write("this bar") }
  this baz { io.write("this baz") }
  this bar(arg) { io.write("this bar " + arg) }

  toString { "Foo" }
}

Foo.bar // expect: this bar
Foo.baz // expect: this baz
Foo.bar("one") // expect: this bar one

// returns the new instance
var foo = Foo.bar // expect: this bar
io.write(foo is Foo) // expect: true
io.write(foo.toString) // expect: Foo
