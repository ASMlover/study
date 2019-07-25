
IO.print(Numeric == Numeric) // expect: true
IO.print(Numeric == Bool) // expect: false

// not equal to other types
IO.print(Numeric == 123) // expect: false
IO.print(Numeric == true) // expect: false

IO.print(Numeric != Numeric) // expect: false
IO.print(Numeric != Bool) // expect: true

IO.print(Numeric != 123) // expect: true
IO.print(Numeric != true) // expect: true
