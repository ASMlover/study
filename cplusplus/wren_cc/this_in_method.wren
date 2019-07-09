
class Foo {
  bar { return this }
  bar2 { return "bar2" }
}

io.write((new Foo).bar.bar2) // expect: bar2
