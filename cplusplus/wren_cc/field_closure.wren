
class Foo {
  this new() { _field = "Foo._field" }

  closeOverGet {
    return Function.new { _field }
  }

  closeOverSet {
    return Function.new { _field = "new value" }
  }
}

var foo = Foo.new()
IO.print(foo.closeOverGet.call()) // expect: Foo._field
foo.closeOverSet.call()
IO.print(foo.closeOverGet.call()) // expect: new value
