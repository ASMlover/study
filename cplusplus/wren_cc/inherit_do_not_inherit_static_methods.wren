
class Foo {
  static methodOnFoo { IO.print("Foo.methodOnFoo") }
}

class Bar is Foo {}

Bar.methodOnFoo // expect runtime error
