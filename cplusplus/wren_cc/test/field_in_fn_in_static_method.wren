
class Foo {
  static bar {
    Function.new { _field = "wat" } // expect error
  }
}
