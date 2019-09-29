
class Foo {
  construct base() {}
}

class Bar is Foo {}

Bar.base() // expect runtime error
