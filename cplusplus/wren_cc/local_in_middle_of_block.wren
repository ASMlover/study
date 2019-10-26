
class Foo {
  bar {
    var a = "aaa"
    IO.print(a) // expect: `aaa`

    var b = a + "bbb"
    IO.print(b) // expect: `aaabbb`

    var c = a + "ccc"
    IO.print(c) // expect: `aaaccc`

    var d = b + "ddd"
    IO.print(d) // expect: `aaabbbddd`
  }
}

(new Foo).bar
