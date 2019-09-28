
class Foo {
  static initialize { __field = "Foo field" }

  static closeOverGet {
    return Function.new { __field }
  }

  static closeOverSet {
    return Function.new { __field = "new value" }
  }
}

Foo.initialize
IO.print(Foo.closeOverGet.call()) // expect: Foo field
Foo.closeOverSet.call()
IO.print(Foo.closeOverGet.call()) // expect: new value
