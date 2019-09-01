
IO.print({}.toString) // expect: {}

IO.print({"1": "2"}.toString) // expect: {1: 2}

// nested maps
IO.print({1: {2: {}}}) // expect: {1: {2: {}}}

// calls toString on elements
class Foo {
  toString { "Foo.toString" }
}
IO.print({1: new Foo}) // expect: {1: Foo.toString}

var s = {1: 2, 3: 4, 5: 6}.toString
IO.print(s == "{1: 2, 3: 4, 5: 6}" ||
         s == "{1: 2, 5: 6, 3: 4}" ||
         s == "{3: 4, 1: 2, 5: 6}" ||
         s == "{3: 4, 5: 6, 1: 2}" ||
         s == "{5: 6, 1: 2, 3: 4}" ||
         s == "{5: 6, 3: 4, 1: 2}") // expect: true
