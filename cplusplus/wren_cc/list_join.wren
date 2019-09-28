
// handle empty list
IO.print([].join(",") == "") // expect: true

// handle a simple list with an empty delimeter
IO.print([1, 2, 3].join("")) // expect: 123

// handle a simple list with no separator
IO.print([1, 2, 3].join) // expect: 123

// does not quote strings
IO.print([1, "2", true].join(",")) // expect: 1,2,true

// nested lists
IO.print([1, [2, [3], 4], 5].join(",")) // expect: 1,[2, [3], 4],5

// calls toString on elements
class Foo {
  toString { "Foo.toString" }
}
IO.print([1, Foo.new(), 2].join(", ")) // expect: 1, Foo.toString, 2
