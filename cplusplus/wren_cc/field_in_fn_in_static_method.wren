
class Foo {
  static bar {
    new Function { _field = "wat" } // expect error
  }
}
