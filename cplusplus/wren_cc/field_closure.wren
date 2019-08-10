
class Foo {
  new { _field = "Foo._field" }

  closeOverGet {
    return new Function { return _field }
  }

  closeOverSet {
    return new Function { _field = "new value" }
  }
}

var foo = new Foo
IO.print(foo.closeOverGet.call) // expect: Foo._field
foo.closeOverSet.call
IO.print(foo.closeOverGet.call) // expect: new value
