
class Outer {
  foo {
    class Inner {
      static bar {
        _field = "nope" // expect error
      }
    }
  }
}
