
class Foo {
  bar {
    var a = "aaa"
    IO.write(a) // expect: `aaa`

    var b = a + "bbb"
    IO.write(b) // expect: `aaabbb`

    var c = a + "ccc"
    IO.write(c) // expect: `aaaccc`

    var d = b + "ddd"
    IO.write(d) // expect: `aaabbbddd`
  }
}

(new Foo).bar
