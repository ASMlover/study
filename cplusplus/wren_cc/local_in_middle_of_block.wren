
class Foo {
  bar {
    var a = "aaa"
    io.write(a) // expect: `aaa`

    var b = a + "bbb"
    io.write(b) // expect: `aaabbb`

    var c = a + "ccc"
    io.write(c) // expect: `aaaccc`

    var d = b + "ddd"
    io.write(d) // expect: `aaabbbddd`
  }
}

(new Foo).bar
