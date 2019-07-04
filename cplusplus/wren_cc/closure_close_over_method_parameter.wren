
var f = nil

class Foo {
  method(param) {
    f = fn {
      io.write(param)
    }
  }
}

Foo.new.method("parameter")
f.call // expect: parameter
