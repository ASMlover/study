
class Foo {
  construct new() {}
}

Foo.new().someUnknownMethod(1) // expect runtime error
