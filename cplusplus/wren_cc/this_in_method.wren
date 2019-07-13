
class Foo {
  bar { return this }
  bar2 { return "bar2" }
}

IO.write((new Foo).bar.bar2) // expect: bar2
