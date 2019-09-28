
class Foo {
  bar { this }
  bar2 { "bar2" }
}

IO.print(Foo.new().bar.bar2) // expect: bar2
