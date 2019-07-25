
class Foo {
  bar { return this }
  bar2 { return "bar2" }
}

IO.print((new Foo).bar.bar2) // expect: bar2
