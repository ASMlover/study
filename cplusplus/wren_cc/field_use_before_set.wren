
class Foo {
  write { io.write(_field) }
  init { _field = "field-value" }
}

var foo = new Foo
foo.init
foo.write // expect: field-value
