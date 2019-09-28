
class Foo {
  this base() {}
}

class Bar is Foo {}

Bar.base() // expect runtime error
