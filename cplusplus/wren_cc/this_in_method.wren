
class Foo {
  bar { this }
  bar2 { "bar2" }
}

io.write(Foo.new.bar.bar2) // expect: bar2
