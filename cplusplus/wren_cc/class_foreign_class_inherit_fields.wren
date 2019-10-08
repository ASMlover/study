
class Foo {
  method() {
    _field = "value"
  }
}

foreign class Bar is Foo {} // expect runtime error
