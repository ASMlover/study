
class Foo {
  toString { "Foo.toString" }
}

// calls toString on argument
IO.print(Foo.new()) // expect: Foo.toString

// with one argument, returns the argument
IO.print(IO.print(1) == 1)
// expect: 1
// expect: true

// with more than one argument, returns nil
IO.print(IO.print(1, 2) == nil)
// expect: 12
// expect: true

// allow multiple arguments and concatenates them
IO.print(1) // expect: 1
IO.print(1,2) // expect: 12
IO.print(1,2,3) // expect: 123
IO.print(1,2,3,4) // expect: 1234
IO.print(1,2,3,4,5) // expect: 12345
IO.print(1,2,3,4,5,6) // expect: 123456
IO.print(1,2,3,4,5,6,7) // expect: 1234567
IO.print(1,2,3,4,5,6,7,8) // expect: 12345678
IO.print(1,2,3,4,5,6,7,8,9) // expect: 123456789
IO.print(1,2,3,4,5,6,7,8,9,10) // expect: 12345678910
IO.print(1,2,3,4,5,6,7,8,9,10,11) // expect: 1234567891011
IO.print(1,2,3,4,5,6,7,8,9,10,11,12) // expect: 123456789101112
IO.print(1,2,3,4,5,6,7,8,9,10,11,12,13) // expect: 12345678910111213
IO.print(1,2,3,4,5,6,7,8,9,10,11,12,13,14) // expect: 1234567891011121314
IO.print(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15) // expect: 123456789101112131415
IO.print(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16) // expect: 12345678910111213141516
