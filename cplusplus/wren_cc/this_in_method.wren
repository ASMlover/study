
class Foo {
  bar { this }
  bar2 { "bar2" }
}

IO.print((new Foo).bar.bar2) // expect: bar2
