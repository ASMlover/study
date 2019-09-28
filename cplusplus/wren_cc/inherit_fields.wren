
class Foo {
  foo(a, b) {
    _field1 = a
    _field2 = b
  }

  fooPrint {
    IO.print(_field1)
    IO.print(_field2)
  }
}

class Bar is Foo {
  bar(a, b) {
    _field1 = a
    _field2 = b
  }

  barPrint {
    IO.print(_field1)
    IO.print(_field2)
  }
}

var bar = Bar.new()
bar.foo("foo1", "foo2")
bar.bar("bar1", "bar2")

bar.fooPrint
// expect: foo1
// expect: foo2

bar.barPrint
// expect: bar1
// expect: bar2
