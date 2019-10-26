
foreign class Foo {
  bar {
    // cannot read a field
    IO.print(_bar) // expect error

    // or write one
    _bar = "value" // expect error
  }
}
