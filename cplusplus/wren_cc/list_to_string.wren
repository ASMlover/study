
IO.write([].toString) // expect: []

// does not quote strings
IO.write([1, "2", true].toString) // expect: [1, 2, true]

// nested lists
IO.write([1, [2, [3], 4], 5]) // expect: [1, [2, [3], 4], 5]

// calls toString on elements
class Foo {
  toString { return "Foo.toString" }
}
IO.write([1, new Foo, 2]) // expect: [1, Foo.toString, 2]
