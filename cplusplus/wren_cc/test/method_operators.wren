
class Foo {
  construct new() {}
  +(other) { "infix + " + other }
  -(other) { "infix - " + other }
  *(other) { "infix * " + other }
  /(other) { "infix / " + other }
  %(other) { "infix % " + other }
  <(other) { "infix < " + other }
  >(other) { "infix > " + other }
  <=(other) { "infix <= " + other }
  >=(other) { "infix >= " + other }
  ==(other) { "infix == " + other }
  !=(other) { "infix != " + other }
  &(other) { "infix & " + other }
  |(other) { "infix | " + other }

  ! { "prefix !" }
  - { "prefix -" }
}

var foo = Foo.new()
IO.print(foo + "a") // expect: infix + a
IO.print(foo - "a") // expect: infix - a
IO.print(foo * "a") // expect: infix * a
IO.print(foo / "a") // expect: infix / a
IO.print(foo % "a") // expect: infix % a
IO.print(foo < "a") // expect: infix < a
IO.print(foo > "a") // expect: infix > a
IO.print(foo <= "a") // expect: infix <= a
IO.print(foo >= "a") // expect: infix >= a
IO.print(foo == "a") // expect: infix == a
IO.print(foo != "a") // expect: infix != a
IO.print(foo & "a") // expect: infix & a
IO.print(foo | "a") // expect: infix | a
IO.print(!foo) // expect: prefix !
IO.print(-foo) // expect: prefix -
