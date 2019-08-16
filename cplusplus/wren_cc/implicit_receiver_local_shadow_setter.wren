
class Foo {
  bar=(value) {
    IO.print("setter")
    return value
  }

  test {
    bar = "value" // expect: setter

    {
      bar = "value" // expect: setter
      var bar = "local"
      bar = "value" // no expectation
    }

    bar = "value" // expect: setter
  }
}

(new Foo).test
